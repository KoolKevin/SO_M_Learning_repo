package main

import (
	"fmt"
	"math/rand"
	"time"
)

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

type dati_utente struct {
	id   int
	tipo string
	ack  chan bool
}

type stato_gestore struct {
	visitatori_in_sala   int
	persone_entranti     int
	persone_uscenti      int
	scolaresche_entranti int
	scolaresche_uscenti  int
	addetti_in_sala      int
	fine                 bool
}

// canali usati dai veicoli per richiedere accesso alla strada
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

func cond_sinc_entrata_singolo(stato stato_gestore, addetti_in_coda int) bool {
	// condizioni di sincronizzazione
	if stato.visitatori_in_sala >= CAPACITÀ_SALA {
		fmt.Printf("\t[VISITATORE]: non entra dato che non c'è spazio in sala!\n")
		return false
	}

	if stato.persone_entranti+stato.persone_uscenti >= CAPACITÀ_CORRIDOIO {
		fmt.Printf("\t[VISITATORE] non entra dato che non c'è spazio in corridoio!\n")
		return false
	}

	if stato.addetti_in_sala == 0 {
		fmt.Printf("\t[VISITATORE] non entra dato che non ci sono addetti in sala!\n")
		return false
	}

	if stato.scolaresche_uscenti > 0 {
		fmt.Printf("\t[VISITATORE] non entra dato che c'è una scolaresca in uscita nel corridoio!\n")
		return false
	}

	// condizioni di priorità
	if stato.persone_uscenti > 0 {
		fmt.Printf("\t[VISITATORE] non entra dato che ci sono persone che stanno uscendo!\n")
		return false
	}

	if addetti_in_coda > 0 {
		fmt.Printf("\t[VISITATORE] non entra dato che vuole entrare un addetto!\n")
		return false
	}

	return true
}

func cond_sinc_uscita_singolo(stato stato_gestore, scolaresche_in_coda int) bool {
	// condizioni di sincronizzazione
	if stato.persone_entranti+stato.persone_uscenti >= CAPACITÀ_CORRIDOIO {
		fmt.Printf("\t[VISITATORE] non esce dato che non c'è spazio in corridoio!\n")
		return false
	}

	if stato.scolaresche_entranti > 0 {
		fmt.Printf("\t[VISITATORE] non entra dato che c'è una scolaresca in entrata nel corridoio!\n")
		return false
	}

	// condizioni di priorità
	if scolaresche_in_coda > 0 {
		fmt.Printf("\t[VISITATORE] non entra dato che vuole entrare una scolaresca!\n")
		return false
	}

	return true
}

func cond_sinc_entrata_scolaresca(stato stato_gestore, più_prio_in_coda int) bool {
	// condizioni di sincronizzazione
	if stato.visitatori_in_sala >= CAPACITÀ_SALA-PERSONE_PER_SCOLARESCA+1 {
		fmt.Printf("\t[SCOLARESCA]: non entra dato che non c'è spazio in sala!\n")
		return false
	}

	if stato.persone_entranti+stato.persone_uscenti >= CAPACITÀ_CORRIDOIO-PERSONE_PER_SCOLARESCA+1 {
		fmt.Printf("\t[SCOLARESCA] non entra dato che non c'è spazio in corridoio!\n")
		return false
	}

	if stato.addetti_in_sala == 0 {
		fmt.Printf("\t[SCOLARESCA] non entra dato che non ci sono addetti in sala!\n")
		return false
	}

	if stato.scolaresche_uscenti > 0 {
		fmt.Printf("\t[SCOLARESCA] non entra dato che c'è una scolaresca in uscita nel corridoio!\n")
		return false
	}

	// condizioni di priorità
	if stato.persone_uscenti > 0 {
		fmt.Printf("\t[SCOLARESCA] non entra dato che ci sono persone che stanno uscendo!\n")
		return false
	}

	if più_prio_in_coda > 0 {
		fmt.Printf("\t[SCOLARESCA] non entra dato che vuole entrare un addetto/singolo!\n")
		return false
	}

	return true
}

func cond_sinc_uscita_scolaresca(stato stato_gestore) bool {
	// condizioni di sincronizzazione
	if stato.persone_entranti+stato.persone_uscenti >= CAPACITÀ_CORRIDOIO-PERSONE_PER_SCOLARESCA+1 {
		fmt.Printf("\t[SCOLARESCA] non esce dato che non c'è spazio in corridoio!\n")
		return false
	}

	if stato.scolaresche_entranti > 0 {
		fmt.Printf("\t[SCOLARESCA] non entra dato che c'è una scolaresca in entrata nel corridoio!\n")
		return false
	}

	// condizioni di priorità
	// scolaresca ha priorità max in uscita

	return true
}

func cond_sinc_entrata_addetto(stato stato_gestore) bool {
	// condizioni di sincronizzazione
	if stato.addetti_in_sala >= MAX_SORVEGLIANTI_IN_SALA {
		fmt.Printf("\t[ADDETTO] non entra dato che ce ne sono già abbastanza in sala!\n")
		return false
	}

	if stato.persone_entranti+stato.persone_uscenti >= CAPACITÀ_CORRIDOIO {
		fmt.Printf("\t[ADDETTO] non entra dato che non c'è spazio in corridoio!\n")
		return false
	}

	if stato.scolaresche_uscenti > 0 {
		fmt.Printf("\t[ADDETTO] non entra dato che c'è una scolaresca in uscita nel corridoio!\n")
		return false
	}

	// condizioni di priorità
	if stato.persone_uscenti > 0 {
		fmt.Printf("\t[ADDETTO] non entra dato che ci sono persone che stanno uscendo!\n")
		return false
	}
	// gli addetti hanno priorità max in ingresso

	return true
}

func cond_sinc_uscita_addetto(stato stato_gestore, più_prio_in_coda int) bool {
	// condizioni di sincronizzazione
	if stato.persone_entranti+stato.persone_uscenti >= CAPACITÀ_CORRIDOIO {
		fmt.Printf("\t[ADDETTO] non esce dato che non c'è spazio in corridoio!\n")
		return false
	}

	if stato.addetti_in_sala == 1 && stato.visitatori_in_sala > 0 {
		fmt.Printf("\t[ADDETTO] non esce dato che ci sono visitatori in sala e lui è l'unico sorvegliante!\n")
		return false
	}

	if stato.scolaresche_entranti > 0 {
		fmt.Printf("\t[ADDETTO] non entra dato che c'è una scolaresca in entrata nel corridoio!\n")
		return false
	}

	// condizioni di priorità
	if più_prio_in_coda > 0 {
		fmt.Printf("\t[ADDETTO] non entra dato che ci sono singoli/scolaresche che stanno uscendo!\n")
		return false
	}

	return true
}

func mostra() {
	// stato mostra
	var stato stato_gestore
	stato.visitatori_in_sala = 0
	stato.persone_entranti = 0
	stato.persone_uscenti = 0
	stato.scolaresche_entranti = 0
	stato.scolaresche_uscenti = 0
	stato.addetti_in_sala = 0
	stato.fine = false

	for {
		select {
		// canali dei visitatori | caso singolo
		case req := <-when_dati_utente(cond_sinc_entrata_singolo(stato, len(entrata_chans[ADDETTO])), entrata_chans[SINGOLO]):
			stato.visitatori_in_sala++ // assumo che un visitatore possa entrare nel corridoio solo se c'è posto in sala
			stato.persone_entranti++
			req.ack <- true
			fmt.Printf("[MOSTRA] il visitatore: (%d; %s) è entrato nel corridoio verso la mostra\n", req.id, req.tipo)

		case req := <-fine_entrata_chans[SINGOLO]:
			stato.persone_entranti--
			fmt.Printf("[MOSTRA] il visitatore: (%d; %s), è entranto nella mostra\n", req.id, req.tipo)

		case req := <-when_dati_utente(cond_sinc_uscita_singolo(stato, len(uscita_chans[SCOLARESCA])), uscita_chans[SINGOLO]):
			stato.visitatori_in_sala--
			stato.persone_uscenti++
			req.ack <- true
			fmt.Printf("[MOSTRA] il visitatore: (%d; %s) è entrato nel corridoio verso casa\n", req.id, req.tipo)

		case req := <-fine_uscita_chans[SINGOLO]:
			stato.persone_uscenti--
			fmt.Printf("[MOSTRA] il visitatore: (%d; %s), è andato via\n", req.id, req.tipo)

		// canali dei visitatori | caso scolaresca
		case req := <-when_dati_utente(cond_sinc_entrata_scolaresca(stato, len(entrata_chans[ADDETTO])+len(entrata_chans[SINGOLO])), entrata_chans[SCOLARESCA]):
			stato.visitatori_in_sala += PERSONE_PER_SCOLARESCA // assumo che una scolaresca possa entrare nel corridoio solo se c'è posto in sala
			stato.persone_entranti += PERSONE_PER_SCOLARESCA
			stato.scolaresche_entranti++
			req.ack <- true
			fmt.Printf("[MOSTRA] il visitatore: (%d; %s) è entrato nel corridoio verso la mostra\n", req.id, req.tipo)

		case req := <-fine_entrata_chans[SCOLARESCA]:
			stato.persone_entranti -= PERSONE_PER_SCOLARESCA
			stato.scolaresche_entranti--
			fmt.Printf("[MOSTRA] il visitatore: (%d; %s), è entranto nella mostra\n", req.id, req.tipo)

		case req := <-when_dati_utente(cond_sinc_uscita_scolaresca(stato), uscita_chans[SCOLARESCA]):
			stato.visitatori_in_sala -= PERSONE_PER_SCOLARESCA
			stato.persone_uscenti += PERSONE_PER_SCOLARESCA
			stato.scolaresche_uscenti++
			req.ack <- true
			fmt.Printf("[MOSTRA] il visitatore: (%d; %s) è entrato nel corridoio verso casa\n", req.id, req.tipo)

		case req := <-fine_uscita_chans[SCOLARESCA]:
			stato.persone_uscenti -= PERSONE_PER_SCOLARESCA
			stato.scolaresche_uscenti--
			fmt.Printf("[MOSTRA] il visitatore: (%d; %s), è andato via\n", req.id, req.tipo)

		// canali degli addetti
		case req := <-when_dati_utente(cond_sinc_entrata_addetto(stato), entrata_chans[ADDETTO]):
			stato.persone_entranti++
			if stato.fine {
				req.ack <- false
			} else {
				req.ack <- true
			}
			fmt.Printf("[MOSTRA] l'addetto %d; è entrato nel corridoio verso la mostra\n", req.id)

		case req := <-fine_entrata_chans[ADDETTO]:
			stato.persone_entranti--
			stato.addetti_in_sala++
			fmt.Printf("[MOSTRA] l'addetto %d è entranto nella mostra a sorvegliare\n", req.id)

		case req := <-when_dati_utente(cond_sinc_uscita_addetto(stato, len(uscita_chans[SINGOLO])+len(uscita_chans[SCOLARESCA])), uscita_chans[ADDETTO]):
			stato.persone_uscenti++
			stato.addetti_in_sala--
			req.ack <- true
			fmt.Printf("[MOSTRA] l'addetto %d è entrato nel corridoio verso una pausa\n", req.id)

		case req := <-fine_uscita_chans[ADDETTO]:
			stato.persone_uscenti--
			fmt.Printf("[MOSTRA] l'addetto %d è uscito dal corridoio verso una pausa\n", req.id)

		// terminazione
		case <-termina_addetti:
			stato.fine = true

		case <-termina:
			fmt.Println("FINE !!!!!!")
			done <- true
			return
		}
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
