package main

import (
	"fmt"
	"math/rand"
	"time"
)

/*
COSTANTI DEL PROBLEMA
*/
const NUM_VISITATORI = 20
const NUM_ADDETTI = 3

const CAPACITÀ_SALA = 10
const CAPACITÀ_CORRIDOIO = 4
const MAX_SORVEGLIANTI_IN_SALA = 2
const PERSONE_PER_SCOLARESCA = 3

const (
	ENTRATA = 0
	USCITA  = 1
)

var DIREZIONI = [2]string{"ENTRATA", "USCITA"}

const TIPI_UTENTE = 3
const (
	SINGOLO    = 0
	SCOLARESCA = 1
	ADDETTO    = 2
)

var NOMI_UTENTE = [TIPI_UTENTE]string{"SINGOLO", "SCOLARESCA", "ADDETTO"}

/*
STRUTTURE DATI
*/
type dati_utente struct {
	id   int
	tipo string
	ack  chan bool
}

type stato_gestore struct {
	// dati...
	fine bool
}

/*
CANALI
*/
var entrata_chans [TIPI_UTENTE]chan dati_utente
var uscita_chans [TIPI_UTENTE]chan dati_utente
var fine_entrata_chans [TIPI_UTENTE]chan dati_utente
var fine_uscita_chans [TIPI_UTENTE]chan dati_utente

// var salita_spazzaneve = make(chan bool, 100)
// var discesa_spazzaneve = make(chan bool, 100)
// var fine_salita_spazzaneve = make(chan bool)
// var fine_discesa_spazzaneve = make(chan bool)
// var ack_spazzaneve = make(chan bool)

// canali usati per la terminazione delle goroutine
var done = make(chan bool)
var termina_addetti = make(chan bool)
var termina = make(chan bool)

func when_bool(b bool, c chan bool) chan bool {
	if !b {
		return nil
	}
	return c
}

func when_dati_utente(b bool, c chan dati_utente) chan dati_utente {
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
func addetto(id int) {
	req := dati_utente{id, NOMI_UTENTE[ADDETTO], make(chan bool)}

	sleepRandTime(3)
	fmt.Printf("[ADDETTO %d]: inizia a lavorare!\n", id)

	// Gli addetti lavora continuamente finchè la mostra non termina
	for {
		entrata_chans[ADDETTO] <- req
		ack := <-req.ack
		if !ack {
			fmt.Printf("[ADDETTO %d]: non ci sono più visitatori, termino anche io!\n", id)
			done <- true
			return
		}
		fmt.Printf("[ADDETTO %d]: entra nel corridoio!\n", id)
		time.Sleep(time.Duration(time.Second)) // tempo per attraversare il corridoio
		fine_entrata_chans[ADDETTO] <- req     // esco dalla strada
		fmt.Printf("\t[ADDETTO %d]: sono arrivato alla mostra!\n", id)

		sleepRandTime(3) // sorveglianza della mostra

		uscita_chans[ADDETTO] <- req
		<-req.ack
		fmt.Printf("[ADDETTO %d]: esco!\n", id)
		time.Sleep(time.Duration(time.Second)) // tempo per attraversare il corridoio
		fine_uscita_chans[ADDETTO] <- req      // esco dalla strada
		fmt.Printf("\t[ADDETTO %d]: sono uscita, faccio una pausa!\n", id)
		sleepRandTime(3) // pausa
	}
}

func visitatore(id int, tipo int) {
	req := dati_utente{id, NOMI_UTENTE[tipo], make(chan bool)}

	fmt.Printf("[VISITATORE %d] %s vuole visitare la mostra!\n", req.id, req.tipo)
	entrata_chans[tipo] <- req
	<-req.ack
	fmt.Printf("[VISITATORE %d] %s entra nel corridoio!\n", id, NOMI_UTENTE[tipo])
	time.Sleep(time.Duration(1 * time.Second)) // tempo per attraversare il corridoio
	fine_entrata_chans[tipo] <- req            // esco dal corridoio
	fmt.Printf("[VISITATORE %d]: sono arrivato alla mostra!\n", id)

	sleepRandTime(3) // partecipazione alla mostra

	uscita_chans[tipo] <- req
	<-req.ack
	fmt.Printf("[VISITATORE %d] esco!\n", id)
	time.Sleep(time.Duration(1 * time.Second)) // tempo per attraversare il corridoio
	fine_uscita_chans[tipo] <- req             // esco dal corridoio
	fmt.Printf("[VISITATORE %d]: sono uscito dal corridoio; torno a casa!\n", id)

	done <- true
}

/*
CONDIZIONI DI SINCRONIZZAZIONE
(attenzione a condizioni con stato.fine)
*/
func cond_sinc_entrata_singolo(stato stato_gestore) bool {
	// condizioni di sincronizzazione

	// regole di priorità

	return true
}

func cond_sinc_uscita_singolo(stato stato_gestore, scolaresche_in_coda int) bool {
	// condizioni di sincronizzazione

	// condizioni di priorità

	return true
}

/*
GOROUTINE GESTORE
*/
func mostra() {
	/*
		inizializzazione stato gestore
	*/
	var stato stato_gestore
	// stato.visitatori_in_sala = 0
	stato.fine = false

	for {
		select {
		// canali utente 1
		case req := <-when_dati_utente(cond_sinc_entrata_singolo(stato), entrata_chans[SINGOLO]):
			// stato.visitatori_in_sala++ // assumo che un visitatore possa entrare nel corridoio solo se c'è posto in sala
			// stato.persone_entranti++
			req.ack <- true
			fmt.Printf("[MOSTRA] il visitatore: (%d; %s) è entrato nel corridoio verso la mostra\n", req.id, req.tipo)

		case req := <-fine_entrata_chans[SINGOLO]:
			// stato.persone_entranti--
			fmt.Printf("[MOSTRA] il visitatore: (%d; %s), è entranto nella mostra\n", req.id, req.tipo)

		case req := <-when_dati_utente(cond_sinc_uscita_singolo(stato), uscita_chans[SINGOLO]):
			// stato.visitatori_in_sala--
			// stato.persone_uscenti++
			req.ack <- true
			fmt.Printf("[MOSTRA] il visitatore: (%d; %s) è entrato nel corridoio verso casa\n", req.id, req.tipo)

		case req := <-fine_uscita_chans[SINGOLO]:
			// stato.persone_uscenti--
			fmt.Printf("[MOSTRA] il visitatore: (%d; %s), è andato via\n", req.id, req.tipo)

		// canali utente 2

		// canali utente speciale

		// terminazione
		case <-termina_addetti:
			stato.fine = true

		case <-termina:
			fmt.Println("FINE !!!!!!")
			done <- true
			return
		}

		// debug
		// stampa_stato(stato)
	}
}

func main() {
	rand.Seed(time.Now().Unix())

	//inizializzazione canali
	for i := 0; i < TIPI_UTENTE; i++ {
		// asincroni perchè ho bisogno di usare len() per le politiche di priorità
		entrata_chans[i] = make(chan dati_utente, 100)
		uscita_chans[i] = make(chan dati_utente, 100)
		fine_entrata_chans[i] = make(chan dati_utente)
		fine_uscita_chans[i] = make(chan dati_utente)
	}

	go mostra()

	for i := 0; i < NUM_VISITATORI; i++ {
		tipo := rand.Intn(TIPI_UTENTE - 1) // non considero gli addetti qua
		go visitatore(i, tipo)
	}

	for i := 0; i < NUM_ADDETTI; i++ {
		go addetto(i)
	}

	// attendo la fine dei visitatori
	for i := 0; i < NUM_VISITATORI; i++ {
		<-done
	}

	// termino gli addetti ed attendo la loro fine
	termina_addetti <- true
	for i := 0; i < NUM_ADDETTI; i++ {
		<-done
	}

	// termino la mostra ed il programma
	termina <- true
	<-done

	fmt.Printf("[main] APPLICAZIONE TERMINATA \n")
}
