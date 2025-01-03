package main

import (
	"fmt"
	"math/rand"
	"time"
)

/*
COSTANTI DEL PROBLEMA
*/
const NUM_ADDETTI = 3
const MAX_PRELIEVI = 1
const TIPI_MASCHERINE = 2

const NUM_RIFORNITORI = TIPI_MASCHERINE

var NOMI_RIFORNITORI = [NUM_RIFORNITORI]string{"FFP2", "CHIRURGICA"}

const CAPACITÀ_SCAFFALE_FFP2 = 10
const CAPACITÀ_SCAFFALE_CHIRURGICHE = 10

const NUM_PRELIEVO_FFP2 = 4
const NUM_PRELIEVO_CHIRURGICHE = 3
const NUM_PRELIEVO_MISTE = 2

const TIPI_PRELIEVO = 3
const (
	FFP2       = 0
	CHIRURGICA = 1
	MISTE      = 2
)

var NOMI_PRELIEVO = [TIPI_PRELIEVO]string{"FFP2", "CHIRURGICA", "MISTE"}

/*
STRUTTURE DATI
*/
type dati_addetto struct {
	id   int
	tipo string
	ack  chan bool
}

type dati_rifornitore struct {
	id   int
	tipo string
	ack  chan bool
}

type stato_gestore struct {
	scatole_ffp2                   int
	scatole_chirurgiche            int
	addetti_presso_ffp2            int
	addetti_presso_chirurgiche     int
	rifornitori_presso_ffp2        int
	rifornitori_presso_chirurgiche int
	fine                           bool
}

/*
CANALI
*/
var prelievo_chans [TIPI_PRELIEVO]chan dati_addetto
var fine_prelievo_chans [TIPI_PRELIEVO]chan dati_addetto
var rifornimento_chans [NUM_RIFORNITORI]chan dati_rifornitore
var fine_rifornimento_chans [NUM_RIFORNITORI]chan dati_rifornitore

// canali usati per la terminazione delle goroutine
var done = make(chan bool)
var termina_rifornitori = make(chan bool)
var termina = make(chan bool)

func when_bool(b bool, c chan bool) chan bool {
	if !b {
		return nil
	}
	return c
}

func when_dati_addetto(b bool, c chan dati_addetto) chan dati_addetto {
	if !b {
		return nil
	}
	return c
}

func when_dati_rifornitore(b bool, c chan dati_rifornitore) chan dati_rifornitore {
	if !b {
		return nil
	}
	return c
}

func sleepRandTime(timeLimit int) {
	if timeLimit > 0 {
		time.Sleep(time.Duration(rand.Intn(timeLimit)+1) * time.Second)
	}
}

func stampa_stato(stato stato_gestore) {

}

/*
GOROUTINE DEGLI UTENTI
*/
func addetto(id int, tipo int) {
	conta_prelievi := 0
	req := dati_addetto{id, NOMI_PRELIEVO[tipo], make(chan bool)}
	sleepRandTime(3)
	fmt.Printf("[ADDETTO %d]: inizia a lavorare!\n", id)

	// Gli addetti lavorano finchè non hanno fatto MAX_PRELIEVI prelievi
	for conta_prelievi < MAX_PRELIEVI {
		prelievo_chans[tipo] <- req
		<-req.ack
		fmt.Printf("[ADDETTO %d]: comincia ad effettuare il suo prelievo!\n", id)

		time.Sleep(time.Duration(time.Second)) // tempo per prelievo

		fine_prelievo_chans[tipo] <- req
		fmt.Printf("[ADDETTO %d]: ha terminato il suo prelievo!\n", id)

		conta_prelievi++
		sleepRandTime(5) // tempo tra cui servirà un nuovo prelievo
	}

	fmt.Printf("--- [ADDETTO %d]: se ne va a casa!\n", id)
	done <- true
}

func rifornitore(id int, tipo int) {
	req := dati_rifornitore{id, NOMI_RIFORNITORI[tipo], make(chan bool)}

	sleepRandTime(3)
	fmt.Printf("[RIFORNITORE %d]: inizia a lavorare!\n", id)

	// I Rifornitori lavorano continuamente finchè qualcuno non gli dice di terminare
	for {
		rifornimento_chans[tipo] <- req
		ack := <-req.ack
		if !ack {
			fmt.Printf("--- [RIFORNITORE %d]: non ci sono più addetti, termino anche io!\n", id)
			done <- true
			return
		}
		fmt.Printf("[RIFORNITORE %d]: comincia il rifornimento dello scaffale %s!\n", id, NOMI_RIFORNITORI[tipo])

		time.Sleep(time.Duration(time.Second)) // tempo per rifornire lo scaffale

		fine_rifornimento_chans[tipo] <- req
		fmt.Printf("[RIFORNITORE %d]: ho terminato il rifornimento!\n", id)

		sleepRandTime(3) // tempo prima del prossimo rifornimento
	}
}

/*
CONDIZIONI DI SINCRONIZZAZIONE
*/
func cond_sinc_prelievo_ffp2(stato stato_gestore, misto_in_coda int) bool {
	// condizioni di sincronizzazione
	if stato.scatole_ffp2 < NUM_PRELIEVO_FFP2 {
		fmt.Printf("\tAddetto aspetta dato che non ci sono abbastanza scatole ffp2\n")
		return false
	}

	if stato.rifornitori_presso_ffp2 > 0 {
		fmt.Printf("\tAddetto aspetta dato che un rifornitore sta lavorando allo scaffale delle ffp2\n")
		return false
	}

	// regole di priorità
	if misto_in_coda > 0 {
		fmt.Printf("\tAddetto aspetta dato che un c'è un prelievo misto in coda\n")
		return false
	}

	return true
}

func cond_sinc_prelievo_chirurgiche(stato stato_gestore, più_prio_in_coda int) bool {
	// condizioni di sincronizzazione
	if stato.scatole_chirurgiche < NUM_PRELIEVO_CHIRURGICHE {
		fmt.Printf("\tAddetto aspetta dato che non ci sono abbastanza scatole chirurgiche\n")
		return false
	}

	if stato.rifornitori_presso_chirurgiche > 0 {
		fmt.Printf("\tAddetto aspetta dato che un rifornitore sta lavorando allo scaffale delle chirurgiche\n")
		return false
	}

	// regole di priorità
	if più_prio_in_coda > 0 {
		fmt.Printf("\tAddetto aspetta dato che un c'è un prelievo misto/ffp2 in coda\n")
		return false
	}

	return true
}

func cond_sinc_prelievo_miste(stato stato_gestore) bool {
	// condizioni di sincronizzazione
	if stato.scatole_ffp2 < NUM_PRELIEVO_MISTE || stato.scatole_chirurgiche < NUM_PRELIEVO_MISTE {
		fmt.Printf("\tAddetto aspetta dato che non ci sono abbastanza scatole per un prelievo misto\n")
		return false
	}

	if stato.rifornitori_presso_ffp2 > 0 || stato.rifornitori_presso_chirurgiche > 0 {
		fmt.Printf("\tAddetto aspetta dato che un rifornitore sta lavorando allo scaffale delle chirurgiche\n")
		return false
	}

	// regole di priorità
	// il prelievo misto ha priorità max

	return true
}

func cond_sinc_rifornimento_ffp2(stato stato_gestore) bool {
	// condizioni di sincronizzazione
	if stato.fine == true {
		//devo terminare e quindi vado avanti
		return true
	}

	if stato.addetti_presso_ffp2 > 0 {
		fmt.Printf("\tRifornitore aspetta dato che un addetto sta lavorando allo scaffale delle ffp2\n")
		return false
	}

	// condizioni di priorità
	if stato.scatole_chirurgiche < stato.scatole_ffp2 {
		fmt.Printf("\tRifornitore aspetta dato che lo scaffale delle chirurgiche ha precedenza\n")
		return false
	}

	return true
}

func cond_sinc_rifornimento_chirurgiche(stato stato_gestore) bool {
	// condizioni di sincronizzazione
	if stato.fine == true {
		//devo terminare e quindi vado avanti
		return true
	}

	if stato.addetti_presso_chirurgiche > 0 {
		fmt.Printf("\tRifornitore aspetta dato che un addetto sta lavorando allo scaffale delle chirurgiche\n")
		return false
	}

	// condizioni di priorità
	if stato.scatole_ffp2 <= stato.scatole_chirurgiche {
		fmt.Printf("\tRifornitore aspetta dato che lo scaffale delle ffp2 ha precedenza\n")
		return false
	}

	return true
}

/*
GOROUTINE GESTORE
*/
func magazzino() {
	var stato stato_gestore
	stato.scatole_ffp2 = CAPACITÀ_SCAFFALE_FFP2
	stato.scatole_chirurgiche = CAPACITÀ_SCAFFALE_CHIRURGICHE
	stato.addetti_presso_ffp2 = 0
	stato.addetti_presso_chirurgiche = 0
	stato.rifornitori_presso_ffp2 = 0
	stato.rifornitori_presso_chirurgiche = 0
	stato.fine = false

	for {
		select {
		// canali addetti
		case req := <-when_dati_addetto(cond_sinc_prelievo_ffp2(stato, len(prelievo_chans[MISTE])), prelievo_chans[FFP2]):
			stato.scatole_ffp2 -= NUM_PRELIEVO_FFP2
			stato.addetti_presso_ffp2++
			req.ack <- true
			fmt.Printf("[MAGAZZINO] L'addetto: (%d; %s) ha iniziato il prelievo\n", req.id, req.tipo)

		case req := <-fine_prelievo_chans[FFP2]:
			stato.addetti_presso_ffp2--
			fmt.Printf("[MAGAZZINO] L'addetto: (%d; %s), ha terminato il prelievo\n", req.id, req.tipo)

		case req := <-when_dati_addetto(cond_sinc_prelievo_chirurgiche(stato, len(prelievo_chans[MISTE])+len(prelievo_chans[FFP2])), prelievo_chans[CHIRURGICA]):
			stato.scatole_chirurgiche -= NUM_PRELIEVO_CHIRURGICHE
			stato.addetti_presso_chirurgiche++
			req.ack <- true
			fmt.Printf("[MAGAZZINO] L'addetto: (%d; %s) ha iniziato il prelievo\n", req.id, req.tipo)

		case req := <-fine_prelievo_chans[CHIRURGICA]:
			stato.addetti_presso_chirurgiche--
			fmt.Printf("[MAGAZZINO] L'addetto: (%d; %s), ha terminato il prelievo\n", req.id, req.tipo)

		case req := <-when_dati_addetto(cond_sinc_prelievo_miste(stato), prelievo_chans[MISTE]):
			stato.scatole_ffp2 -= NUM_PRELIEVO_MISTE
			stato.scatole_chirurgiche -= NUM_PRELIEVO_MISTE
			stato.addetti_presso_ffp2++
			stato.addetti_presso_chirurgiche++
			req.ack <- true
			fmt.Printf("[MAGAZZINO] L'addetto: (%d; %s) ha iniziato il prelievo\n", req.id, req.tipo)

		case req := <-fine_prelievo_chans[MISTE]:
			stato.addetti_presso_ffp2--
			stato.addetti_presso_chirurgiche--
			fmt.Printf("[MAGAZZINO] L'addetto: (%d; %s), ha terminato il prelievo\n", req.id, req.tipo)

		// canali rifornitori
		case req := <-when_dati_rifornitore(cond_sinc_rifornimento_ffp2(stato), rifornimento_chans[FFP2]):
			stato.scatole_ffp2 = CAPACITÀ_SCAFFALE_FFP2
			stato.rifornitori_presso_ffp2++
			if stato.fine {
				req.ack <- false
			} else {
				req.ack <- true
			}
			fmt.Printf("[MAGAZZINO] Il rifornitore: (%d; %s) ha iniziato il rifornimento\n", req.id, req.tipo)

		case req := <-fine_rifornimento_chans[FFP2]:
			stato.rifornitori_presso_ffp2--
			fmt.Printf("[MAGAZZINO] Il rifornitore: (%d; %s), ha terminato il rifornimento\n", req.id, req.tipo)

		case req := <-when_dati_rifornitore(cond_sinc_rifornimento_chirurgiche(stato), rifornimento_chans[CHIRURGICA]):
			stato.scatole_chirurgiche = CAPACITÀ_SCAFFALE_CHIRURGICHE
			stato.rifornitori_presso_chirurgiche++
			if stato.fine {
				req.ack <- false
			} else {
				req.ack <- true
			}
			fmt.Printf("[MAGAZZINO] Il rifornitore: (%d; %s) ha iniziato il rifornimento\n", req.id, req.tipo)

		case req := <-fine_rifornimento_chans[CHIRURGICA]:
			stato.rifornitori_presso_chirurgiche--
			fmt.Printf("[MAGAZZINO] Il rifornitore: (%d; %s), ha terminato il rifornimento\n", req.id, req.tipo)

		// terminazione
		case <-termina_rifornitori:
			stato.fine = true

		case <-termina:
			fmt.Println("FINE !!!!!!")
			done <- true
			return
		}

		// per debug:
		// stampa_stato(stato)
	}
}

func main() {
	rand.Seed(time.Now().Unix())

	//inizializzazione canali
	for i := 0; i < TIPI_PRELIEVO; i++ {
		// asincrono perchè ho bisogno di usare len() per le politiche di priorità
		prelievo_chans[i] = make(chan dati_addetto, 100)
		fine_prelievo_chans[i] = make(chan dati_addetto)
	}
	for i := 0; i < NUM_RIFORNITORI; i++ {
		// asincrono perchè ho bisogno di usare len() per le politiche di priorità
		rifornimento_chans[i] = make(chan dati_rifornitore, 100)
		fine_rifornimento_chans[i] = make(chan dati_rifornitore)
	}

	go magazzino()

	for i := 0; i < NUM_ADDETTI; i++ {
		tipo := rand.Intn(TIPI_PRELIEVO)
		go addetto(i, tipo)
	}

	go rifornitore(0, FFP2)
	go rifornitore(1, CHIRURGICA)

	// attendo la fine degli addetti
	for i := 0; i < NUM_ADDETTI; i++ {
		<-done
	}

	// termino i rifornitori ed attendo la loro fine
	termina_rifornitori <- true
	for i := 0; i < NUM_RIFORNITORI; i++ {
		fmt.Printf("[main] aspetto il rifornitore %d\n", i)
		<-done
	}

	// termino il magazzino ed il programma
	termina <- true
	<-done

	fmt.Printf("[main] APPLICAZIONE TERMINATA \n")
}
