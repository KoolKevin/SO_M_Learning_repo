package main

import (
	"fmt"
	"math/rand"
	"time"
)

const MAX_POSTI_STANDARD = 5
const MAX_POSTI_MAXI = 3
const NUM_CAMPER = 5
const NUM_AUTO = 8

const TIPI_TURISTI = 2
const (
	AUTOMOBILE = 0
	CAMPER     = 1
)

var NOMI_TURISTI = [TIPI_TURISTI]string{"AUTOMOBILE", "CAMPER"}

type req_t struct {
	id   int
	tipo string
	ack  chan bool
}

// canali usati dai veicoli per richiedere accesso alla strada
var salita_chans [TIPI_TURISTI]chan req_t
var discesa_chans [TIPI_TURISTI]chan req_t
var fine_salita_chans [TIPI_TURISTI]chan bool
var fine_discesa_chans [TIPI_TURISTI]chan bool

var salita_spazzaneve = make(chan bool, 100)
var discesa_spazzaneve = make(chan bool, 100)
var fine_salita_spazzaneve = make(chan bool)
var fine_discesa_spazzaneve = make(chan bool)
var ack_spazzaneve = make(chan bool)

// canali usati per la terminazione delle goroutine
var done = make(chan bool)
var termina_spazzaneve = make(chan bool)
var termina = make(chan bool)

func when_bool(b bool, c chan bool) chan bool {
	if !b {
		return nil
	}
	return c
}

func when_req_t(b bool, c chan req_t) chan req_t {
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

func spazzaneve() {
	sleepRandTime(3)
	fmt.Printf("[SPAZZANEVE]: inizia a lavorare!\n")

	// lo spazzaneve lavora continuamente finche qualcuno non gli dice di terminare
	for {
		discesa_spazzaneve <- true // valore arbitraio non importante
		ack := <-ack_spazzaneve
		if !ack {
			fmt.Printf("[SPAZZANEVE]: non ci sono più turisti, termino anche io!\n")
			done <- true
			return
		}
		fmt.Printf("[SPAZZANEVE]: scendo!\n")
		time.Sleep(time.Duration(2 * time.Second)) // tempo di discesa
		fine_discesa_spazzaneve <- true            // esco dalla strada
		fmt.Printf("\t[SPAZZANEVE]: sono arrivato a valle!\n")
		sleepRandTime(3) // sosta al bar

		salita_spazzaneve <- true // valore arbitraio non importante
		<-ack_spazzaneve
		fmt.Printf("[SPAZZANEVE]: salgo!\n")
		time.Sleep(time.Duration(2 * time.Second)) // tempo di salita
		fine_salita_spazzaneve <- true             // esco dalla strada
		fmt.Printf("\t[SPAZZANEVE]: sono arrivato al piazzale!\n")
		sleepRandTime(3) // sosta al piazzale
	}
}

func turista(id int) {
	tipo := rand.Intn(TIPI_TURISTI)
	req := req_t{id, NOMI_TURISTI[tipo], make(chan bool)}

	fmt.Printf("[TURISTA %d] vuole salire con %s!\n", req.id, req.tipo)
	salita_chans[id] <- req
	<-req.ack
	fmt.Printf("[TURISTA %d] salgo!\n")
	time.Sleep(time.Duration(1 * time.Second)) // tempo di salita
	fine_salita_chans[id] <- true              // esco dalla strada
	fmt.Printf("\t[TURISTA %d]: sono arrivato al piazzale!\n", id)
	sleepRandTime(3) // sosta al piazzale

	discesa_chans[id] <- req
	<-req.ack
	fmt.Printf("[TURISTA %d] scendo!\n")
	time.Sleep(time.Duration(1 * time.Second)) // tempo di discesa
	fine_discesa_chans[id] <- true             // esco dalla strada
	fmt.Printf("\t[TURISTA %d]: sono sceso a valle; torno a casa!\n", id)

	done <- true
}

func condizione_sincronizzazione_prelievo_piccolo(quantità_acqua float32, quantità_cassetta_10 int, una_cassetta_piena bool) bool {
	var abbastanza_acqua bool = quantità_acqua >= 0.5

	/*
		Un prelievo piccolo può essere effettuato se:
			- c'è abbastanza acqua
			- la relativa cassetta non è piena
			- non c'è un addetto al lavoro (non controllo dato che è garantito dal canale di fine_intervento)

		Inoltre l'addetto ha priorià se:
			- il serbatoio è vuoto	(non controllo a causa di condizione sopra)
			- OPPURE, una cassetta qualsiasi è piena
	*/
	if quantità_acqua == 0 || una_cassetta_piena { // precedenza all'addetto
		return false
	}

	if abbastanza_acqua && quantità_cassetta_10 != CAPACITÀ_CASSETTA_10 {
		return true
	}

	return false
}

func condizione_sincronizzazione_prelievo_grande(quantità_acqua float32, quantità_cassetta_20 int, una_cassetta_piena bool, num_prelievo_piccolo int) bool {
	var abbastanza_acqua bool = quantità_acqua >= 1.5

	/*
		Un prelievo grande può essere effettuato se:
			- c'è abbastanza acqua
			- la relativa cassetta non è piena(non controllo a causa di condizione sotto)
			- non c'è un addetto al lavoro
			- non ci sono persone in coda che vogliono fare un prelievo piccolo

		Inoltre, l'addetto ha priorià se:
			- il serbatoio è vuoto	(non controllo a causa di condizione sopra)
			- OPPURE, una cassetta qualsiasi è piena
	*/

	// precedenza all'addetto
	if quantità_acqua == 0 || una_cassetta_piena {
		return false
	}

	if abbastanza_acqua && quantità_cassetta_20 != CAPACITÀ_CASSETTA_20 && num_prelievo_piccolo == 0 {
		return true
	}

	return false
}

func condizione_sincronizzazione_addetto(quantità_acqua float32, una_cassetta_piena bool, prelievi_in_corso int) bool {
	/*
		L'addetto ha priorià se:
			- il serbatoio è vuoto	(non controllo a causa di condizione sopra)
			- OPPURE, una cassetta qualsiasi è piena

		L'addetto aspetta se:
			- ci sono cittadini che vogliono fare un prelievo
	*/
	if quantità_acqua == 0 || una_cassetta_piena {
		return true
	}

	if prelievi_in_corso == 0 {
		return true
	}

	return false
}

func strada() {
	var id int
	// stato erogatore
	var quantità_acqua float32 = CAPACITÀ_SERBATOIO
	var quantità_cassetta_10 int = 0
	var quantità_cassetta_20 int = 0
	var una_cassetta_piena bool = false

	// var fine bool = false // diventa true quando sono stati completati i montaggi di TOT auto

	for {
		select {
		// canali dei cittadini
		case id = <-when(condizione_sincronizzazione_prelievo_piccolo(quantità_acqua, quantità_cassetta_10, una_cassetta_piena), prelievo_piccolo):
			quantità_acqua -= 0.5
			quantità_cassetta_10++
			// erogazione
			time.Sleep(time.Duration(1) * time.Second)
			// risposta
			eroga_piccolo <- 1
			fmt.Printf("[erogatore]: servito un prelievo piccolo del cittadino %d;\n\tstato: (%f, %d, %d)", id, quantità_acqua, quantità_cassetta_10, quantità_cassetta_20)

		case id = <-when(condizione_sincronizzazione_prelievo_grande(quantità_acqua, quantità_cassetta_20, una_cassetta_piena, len(prelievo_piccolo)), prelievo_grande):
			quantità_acqua -= 1.5
			quantità_cassetta_20++
			// erogazione
			time.Sleep(time.Duration(2) * time.Second)
			// risposta
			eroga_grande <- 1
			fmt.Printf("[erogatore]: servito un prelievo grande del cittadino %d;\n\tstato: (%f, %d, %d)", id, quantità_acqua, quantità_cassetta_10, quantità_cassetta_20)

		// canale dell'addetto
		case <-when(condizione_sincronizzazione_addetto(quantità_acqua, una_cassetta_piena, len(prelievo_piccolo)+len(prelievo_grande)), req_intervento):
			ok_intervento <- 1

			quantità_acqua = CAPACITÀ_SERBATOIO
			quantità_cassetta_10 = 0
			quantità_cassetta_20 = 0
			// aspetto il termine dell'intervento
			<-fine_intervento
			fmt.Printf("[erogatore]: l'addetto ha finito il suo intervento;\n\tstato: (%f, %d, %d)", quantità_acqua, quantità_cassetta_10, quantità_cassetta_20)

			//terminazione
			// case <-terminaDeposito:
			// 	fmt.Printf("[erogatore]: termino\n")
			// 	done <- true
			// 	return
		}

		una_cassetta_piena = (quantità_cassetta_10 == CAPACITÀ_CASSETTA_10) || (quantità_cassetta_20 == CAPACITÀ_CASSETTA_20)
	}
}

func main() {
	rand.Seed(time.Now().Unix())

	//inizializzazione canali
	for i := 0; i < TIPI_TURISTI; i++ {
		// asincroni perchè ho bisogno di usare len() per le politiche di priorità
		salita_chans[i] = make(chan req_t, 100)
		discesa_chans[i] = make(chan req_t, 100)
		fine_salita_chans[i] = make(chan bool)
		fine_discesa_chans[i] = make(chan bool)
	}

	go erogatore()

	for i := 0; i < CITTADINI_CON_RICHIESTE_GRANDI; i++ {
		var req cittadino_req_t
		req.id_cittadino = i
		req.tipo = tipi_richiesta[1]

		go cittadino(req)
	}

	for i := 0; i < CITTADINI_CON_RICHIESTE_PICCOLE; i++ {
		var req cittadino_req_t
		req.id_cittadino = i + CITTADINI_CON_RICHIESTE_GRANDI
		req.tipo = tipi_richiesta[0]

		go cittadino(req)
	}

	go addetto()

	// //attendo la fine dei turisti
	// for i := 0; i < MAXPROC; i++ {
	// 	<-done
	// }

	// terminaAddetto <- true
	// <-done

	// termina_erogatore <- true
	// <-done

	// fmt.Printf("[main] APPLICAZIONE TERMINATA \n")
}
