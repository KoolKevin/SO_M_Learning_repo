package main

import (
	"fmt"
	"math/rand"
	"time"
)

const MAX_POSTI_STANDARD = 5
const MAX_POSTI_MAXI = 3
const NUM_TURISTI = 10

const TIPI_TURISTI = 2
const (
	AUTOMOBILE = 0
	CAMPER     = 1
)

var NOMI_TURISTI = [TIPI_TURISTI]string{"AUTOMOBILE", "CAMPER"}

const TIPI_POSTI = 3
const (
	NESSUNO  = 0
	STANDARD = 1
	MAXI     = 2
)

var NOMI_POSTI = [TIPI_POSTI]string{"NESSUNO", "STANDARD", "MAXI"}

type dati_turista struct {
	id                  int
	tipo                string
	ack                 chan int // l'ack mi dice anche che tipo di posto è disponibile alla piazzola
	tipo_posto_occupato int      // questo mi serve quando ritorno giù per far capire al gestore che posto ho liberato
}

type stato_gestore struct {
	num_posti_standard     int
	num_posti_maxi         int
	auto_in_salita         int
	auto_in_discesa        int
	camper_in_salita       int
	camper_in_discesa      int
	spazzaneve_in_transito bool
	fine                   bool
}

// canali usati dai veicoli per richiedere accesso alla strada
var salita_chans [TIPI_TURISTI]chan dati_turista
var discesa_chans [TIPI_TURISTI]chan dati_turista
var fine_salita_chans [TIPI_TURISTI]chan dati_turista
var fine_discesa_chans [TIPI_TURISTI]chan dati_turista

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

func when_dati_turista(b bool, c chan dati_turista) chan dati_turista {
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
	req := dati_turista{id, NOMI_TURISTI[tipo], make(chan int), NESSUNO}

	fmt.Printf("[TURISTA %d] vuole salire con %s!\n", req.id, req.tipo)
	salita_chans[tipo] <- req
	var tipo_posto int = <-req.ack
	req.tipo_posto_occupato = tipo_posto
	fmt.Printf("[TURISTA %d] salgo ad occupare %s!\n", id, NOMI_POSTI[tipo_posto])
	time.Sleep(time.Duration(1 * time.Second)) // tempo di salita
	fine_salita_chans[tipo] <- req             // esco dalla strada
	fmt.Printf("\t[TURISTA %d]: sono arrivato al piazzale!\n", id)
	sleepRandTime(3) // sosta al piazzale

	discesa_chans[tipo] <- req
	<-req.ack
	fmt.Printf("[TURISTA %d] scendo!\n", id)
	time.Sleep(time.Duration(1 * time.Second)) // tempo di discesa
	fine_discesa_chans[tipo] <- req            // esco dalla strada
	fmt.Printf("\t[TURISTA %d]: sono sceso a valle; torno a casa!\n", id)

	done <- true
}

func condizione_sinc_auto_salita(stato stato_gestore, num_veicoli_in_discesa int, num_camper_in_salita int) bool {
	// condizioni di sincronizzazione
	if stato.spazzaneve_in_transito {
		return false
	}

	if stato.camper_in_discesa != 0 {
		return false
	}

	if stato.num_posti_standard == 0 && stato.num_posti_maxi == 0 {
		return false
	}

	// condizioni di priorità
	if num_veicoli_in_discesa != 0 || num_camper_in_salita != 0 {
		return false
	}

	return true
}

func condizione_sinc_auto_discesa(stato stato_gestore, num_camper_in_salita int, spazzaneve_in_coda int) bool {
	// condizioni di sincronizzazione
	if stato.spazzaneve_in_transito {
		return false
	}

	if stato.camper_in_salita != 0 {
		return false
	}

	// condizioni di priorità
	if num_camper_in_salita != 0 && spazzaneve_in_coda != 0 {
		return false
	}

	return true
}

func condizione_sinc_camper_salita(stato stato_gestore) bool {
	// condizioni di sincronizzazione
	if stato.spazzaneve_in_transito {
		return false
	}

	if stato.camper_in_discesa != 0 || stato.auto_in_discesa != 0 {
		return false
	}

	if stato.num_posti_maxi == 0 {
		return false
	}

	// condizioni di priorità
	// camper ha priorità max in salita

	return true
}

func condizione_sinc_camper_discesa(stato stato_gestore, spazzaneve_in_coda int) bool {
	// condizioni di sincronizzazione
	if stato.spazzaneve_in_transito {
		return false
	}

	if stato.camper_in_salita != 0 || stato.auto_in_salita != 0 {
		return false
	}

	// condizioni di priorità
	if spazzaneve_in_coda != 0 {
		return false
	}

	return true
}

func condizione_sinc_spazzaneve_salita(stato stato_gestore, num_camper_in_salita int, num_auto_in_salita int) bool {
	// condizioni di sincronizzazione
	if stato.auto_in_discesa != 0 || stato.auto_in_salita != 0 || stato.camper_in_discesa != 0 || stato.camper_in_salita != 0 {
		return false
	}

	// condizioni di priorità
	if num_camper_in_salita != 0 || num_auto_in_salita != 0 {
		return false
	}

	return true
}

func condizione_sinc_spazzaneve_discesa(stato stato_gestore) bool {
	// condizioni di sincronizzazione
	if stato.auto_in_discesa != 0 || stato.auto_in_salita != 0 || stato.camper_in_discesa != 0 || stato.camper_in_salita != 0 {
		return false
	}

	// condizioni di priorità
	// spazzaneve ha priorità max in discesa

	return true
}

func strada() {
	// stato strada/piazzale
	var stato stato_gestore
	stato.num_posti_standard = MAX_POSTI_STANDARD
	stato.num_posti_maxi = MAX_POSTI_MAXI
	stato.auto_in_salita = 0
	stato.auto_in_discesa = 0
	stato.camper_in_salita = 0
	stato.camper_in_discesa = 0
	stato.spazzaneve_in_transito = false
	stato.fine = false

	for {
		select {
		// canali dei turisti | caso salita
		case req := <-when_dati_turista(condizione_sinc_auto_salita(stato, len(discesa_chans[AUTOMOBILE])+len(discesa_chans[CAMPER]),
			len(salita_chans[CAMPER])), salita_chans[AUTOMOBILE]):

			stato.auto_in_salita++
			if stato.num_posti_standard == 0 && stato.num_posti_maxi > 0 {
				stato.num_posti_maxi--
				req.ack <- MAXI
			} else {
				stato.num_posti_standard--
				req.ack <- STANDARD
			}
			fmt.Printf("[STRADA] il turista: %d, sta salendo la strada in: %s\n", req.id, req.tipo)

		case req := <-when_dati_turista(condizione_sinc_camper_salita(stato), salita_chans[CAMPER]):
			stato.camper_in_salita++
			stato.num_posti_maxi--
			req.ack <- MAXI
			fmt.Printf("[STRADA] il turista: %d, sta salendo la strada in: %s\n", req.id, req.tipo)

		case req := <-fine_salita_chans[AUTOMOBILE]:
			stato.auto_in_salita--
			fmt.Printf("[STRADA] il turista: %d, è arrivato al piazzale in: %s\n", req.id, req.tipo)

		case req := <-fine_salita_chans[CAMPER]:
			stato.camper_in_salita--
			fmt.Printf("[STRADA] il turista: %d, è arrivato al piazzale in: %s\n", req.id, req.tipo)

		// canali dei turisti | caso discesa
		case req := <-when_dati_turista(condizione_sinc_auto_discesa(stato, len(salita_chans[CAMPER]), len(discesa_spazzaneve)), discesa_chans[AUTOMOBILE]):
			stato.auto_in_discesa++
			if req.tipo_posto_occupato == MAXI {
				stato.num_posti_maxi++
			} else {
				stato.num_posti_standard++
			}
			req.ack <- 100 // valore arbitrario
			fmt.Printf("[STRADA] il turista: %d, sta scendendo la strada in: %s\n", req.id, req.tipo)

		case req := <-when_dati_turista(condizione_sinc_camper_discesa(stato, len(discesa_spazzaneve)), discesa_chans[CAMPER]):
			stato.camper_in_discesa++
			stato.num_posti_maxi++
			req.ack <- 100 // valore arbitrario
			fmt.Printf("[STRADA] il turista: %d, sta scendendo la strada in: %s\n", req.id, req.tipo)

		case req := <-fine_discesa_chans[AUTOMOBILE]:
			stato.auto_in_discesa--
			fmt.Printf("[STRADA] il turista: %d, è tornato a valle in: %s\n", req.id, req.tipo)

		case req := <-fine_discesa_chans[CAMPER]:
			stato.camper_in_discesa--
			fmt.Printf("[STRADA] il turista: %d, è tornato a valle in: %s\n", req.id, req.tipo)

		// canali dello spazzaneve
		case <-when_bool(condizione_sinc_spazzaneve_discesa(stato), discesa_spazzaneve):
			stato.spazzaneve_in_transito = true
			if stato.fine {
				ack_spazzaneve <- false
			} else {
				ack_spazzaneve <- true
			}

		case <-fine_discesa_spazzaneve:
			stato.spazzaneve_in_transito = false

		case <-when_bool(condizione_sinc_spazzaneve_salita(stato, len(salita_chans[CAMPER]), len(salita_chans[AUTOMOBILE])), salita_spazzaneve):
			stato.spazzaneve_in_transito = true
			ack_spazzaneve <- true

		case <-fine_salita_spazzaneve:
			stato.spazzaneve_in_transito = false

		// terminazione
		case <-termina_spazzaneve:
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
	for i := 0; i < TIPI_TURISTI; i++ {
		// asincroni perchè ho bisogno di usare len() per le politiche di priorità
		salita_chans[i] = make(chan dati_turista, 100)
		discesa_chans[i] = make(chan dati_turista, 100)
		fine_salita_chans[i] = make(chan dati_turista)
		fine_discesa_chans[i] = make(chan dati_turista)
	}

	go strada()

	for i := 0; i < NUM_TURISTI; i++ {
		go turista(i)
	}

	go spazzaneve()

	// attendo la fine dei turisti
	for i := 0; i < NUM_TURISTI; i++ {
		<-done
	}

	// termino lo spazzaneve ed attendo la sua fine
	termina_spazzaneve <- true
	<-done

	// termino la strada ed il programma
	termina <- true
	<-done

	fmt.Printf("[main] APPLICAZIONE TERMINATA \n")
}
