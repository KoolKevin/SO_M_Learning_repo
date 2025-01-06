package main

import (
	"fmt"
	"math/rand"
	"time"
)

/*
COSTANTI DEL PROBLEMA
*/
const NUM_REGIONI = 10

const CAPACITÀ_P = 5
const CAPACITÀ_M = 5
const NUM_DOSI_DEPOSITO = 2
const NUM_DOSI_PRELIEVO = 2
const MAX_CONSEGNE_P = 3
const MAX_CONSEGNE_M = 3

const TIPI_ZONA = 3
const (
	GIALLA    = 0
	ARANCIONE = 1
	ROSSA     = 2
)

var NOMI_ZONA = [TIPI_ZONA]string{"ZONA GIALLA", "ZONA ARANCIONE", "ZONA ROSSA"}

const TIPI_VACCINO = 2
const NUM_CASE_FARM = TIPI_VACCINO
const (
	VACCINO_P = 0
	VACCINO_M = 1
)

var NOMI_VACCINI = [TIPI_ZONA]string{"PFIZER", "MODERNA"}

/*
STRUTTURE DATI
*/

// uso la stessa struttura sia per regioni che per case farmaceutiche
type dati_utente struct {
	id   int
	tipo string
	ack  chan bool
}

type stato_gestore struct {
	dosi_disponibili [TIPI_VACCINO]int
	lotti_consegnati [TIPI_VACCINO]int
	fine             bool
}

/*
CANALI
*/
var prelievo_chans [TIPI_ZONA]chan dati_utente
var deposito_chans [NUM_CASE_FARM]chan dati_utente

// canali usati per la terminazione delle goroutine
var done = make(chan bool)
var termina_deposito = make(chan bool)

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
	// ...
}

/*
GOROUTINE DEGLI UTENTI
*/
func casa_farm(id int) {
	req := dati_utente{id, NOMI_VACCINI[id], make(chan bool)}
	sleepRandTime(3)
	fmt.Printf("[CASA FARMACEUTICA %d]: inizia a lavorare!\n", id)

	// Le case farmaceutiche depositano continuamente finchè il deposito non chiude
	for {
		fmt.Printf("[CASA FARMACEUTICA %d]: vuole effettuare un deposito di %s!\n", id, NOMI_VACCINI[id])
		deposito_chans[id] <- req
		continua := <-req.ack
		if !continua {
			fmt.Printf("[CASA FARMACEUTICA %d]: sono state effettuati abbastanza prelievi, termino!\n", id)
			done <- true
			return
		}

		sleepRandTime(3) // tempo prima del prossimo deposito
	}
}

func regione(id int, zona int) {
	req := dati_utente{id, NOMI_ZONA[zona], make(chan bool)}
	sleepRandTime(3)
	fmt.Printf("[REGIONE (%d, %s)]: inizia a lavorare!\n", id, NOMI_ZONA[zona])

	// Le regioni continuano a prelevera continuamente finchè il deposito non chiude
	for {
		fmt.Printf("[REGIONE (%d, %s)]: vuole effettuare un prelievo!\n", id, NOMI_ZONA[zona])
		prelievo_chans[zona] <- req
		continua := <-req.ack
		if !continua {
			fmt.Printf("[REGIONE (%d, %s)]: sono stati effettuati abbastanza prelievi, termino!\n", id, NOMI_ZONA[zona])
			done <- true
			return
		}
		fmt.Printf("[REGIONE (%d, %s)]: sta effettuando un prelievo!\n", id, NOMI_ZONA[zona])

		sleepRandTime(3) // tempo prima del prossimo prelievo
	}
}

/*
CONDIZIONI DI SINCRONIZZAZIONE
(attenzione a condizioni con stato.fine)
*/
func cond_sinc_prelievo_gialla(stato stato_gestore, più_prio int) bool {
	// condizioni di sincronizzazione
	if stato.fine {
		// se il deposito è in terminazine devo permettere ai suoi clienti di terminare
		return true
	}

	if stato.dosi_disponibili[VACCINO_P]+stato.dosi_disponibili[VACCINO_M] < NUM_DOSI_PRELIEVO {
		// non ci sono abbastanza dosi di vaccino per un prelievo
		return false
	}

	// regole di priorità
	if più_prio > 0 {
		// c'è una regione rossa/arancione in coda
		return false
	}

	return true
}

func cond_sinc_prelievo_arancione(stato stato_gestore, rosse_in_coda int) bool {
	// condizioni di sincronizzazione
	if stato.fine {
		// se il deposito è in terminazine devo permettere ai suoi clienti di terminare
		return true
	}

	if stato.dosi_disponibili[VACCINO_P]+stato.dosi_disponibili[VACCINO_M] < NUM_DOSI_PRELIEVO {
		// non ci sono abbastanza dosi di vaccino per un prelievo
		return false
	}

	// regole di priorità
	if rosse_in_coda > 0 {
		// c'è una regione rossa in coda
		return false
	}

	return true
}

func cond_sinc_prelievo_rossa(stato stato_gestore) bool {
	// condizioni di sincronizzazione
	if stato.fine {
		// se il deposito è in terminazine devo permettere ai suoi clienti di terminare
		return true
	}

	if stato.dosi_disponibili[VACCINO_P]+stato.dosi_disponibili[VACCINO_M] < NUM_DOSI_PRELIEVO {
		// non ci sono abbastanza dosi di vaccino per un prelievo
		return false
	}

	// regole di priorità
	// le regioni rosse hanno priorità max

	return true
}

/*
GOROUTINE GESTORE
*/
func deposito() {
	/*
		inizializzazione stato gestore
	*/
	var stato stato_gestore
	stato.dosi_disponibili[VACCINO_P] = CAPACITÀ_P
	stato.dosi_disponibili[VACCINO_M] = CAPACITÀ_M
	for i := 0; i < TIPI_VACCINO; i++ {
		stato.lotti_consegnati[i] = 0
	}
	stato.fine = false

	for {
		select {
		// canali regioni
		case req := <-when_dati_utente(cond_sinc_prelievo_gialla(stato, len(prelievo_chans[ROSSA])+len(prelievo_chans[ARANCIONE])), prelievo_chans[GIALLA]):
			if stato.fine {
				req.ack <- false
			} else {
				req.ack <- true
			}

			// aggiorno la quantità di lotti consegnati e le dosi di vaccino rimanenti
			if stato.dosi_disponibili[VACCINO_P] >= NUM_DOSI_PRELIEVO {
				stato.dosi_disponibili[VACCINO_P] -= NUM_DOSI_PRELIEVO
				stato.lotti_consegnati[VACCINO_P]++
			} else if stato.dosi_disponibili[VACCINO_M] >= NUM_DOSI_PRELIEVO {
				stato.dosi_disponibili[VACCINO_M] -= NUM_DOSI_PRELIEVO
				stato.lotti_consegnati[VACCINO_M]++
			} else {
				// azzero la quantità più piccola ed al resto ci pensa la quantità più grande
				if stato.dosi_disponibili[VACCINO_P] > stato.dosi_disponibili[VACCINO_M] {
					resto := NUM_DOSI_PRELIEVO - stato.dosi_disponibili[VACCINO_M]
					stato.dosi_disponibili[VACCINO_M] = 0
					stato.dosi_disponibili[VACCINO_P] -= resto
				} else {
					resto := NUM_DOSI_PRELIEVO - stato.dosi_disponibili[VACCINO_P]
					stato.dosi_disponibili[VACCINO_P] = 0
					stato.dosi_disponibili[VACCINO_M] -= resto
				}

				stato.lotti_consegnati[VACCINO_P]++
				stato.lotti_consegnati[VACCINO_M]++
			}

			fmt.Printf("[DEPOSITO] La regione: (%d; %s) ha fatto un prelievo\n", req.id, req.tipo)

		/*
			Le seguenti due guardie sono identiche a sopra.
			Ho deciso di separare i canali e le relative guardie in modo da poter
			gestire le condizione di priorità più agevolmente con 'len()'
		*/
		case req := <-when_dati_utente(cond_sinc_prelievo_arancione(stato, len(prelievo_chans[ROSSA])), prelievo_chans[ARANCIONE]):
			if stato.fine {
				req.ack <- false
			} else {
				req.ack <- true
			}

			// aggiorno la quantità di lotti consegnati e le dosi di vaccino rimanenti
			if stato.dosi_disponibili[VACCINO_P] >= NUM_DOSI_PRELIEVO {
				stato.dosi_disponibili[VACCINO_P] -= NUM_DOSI_PRELIEVO
				stato.lotti_consegnati[VACCINO_P]++
			} else if stato.dosi_disponibili[VACCINO_M] >= NUM_DOSI_PRELIEVO {
				stato.dosi_disponibili[VACCINO_M] -= NUM_DOSI_PRELIEVO
				stato.lotti_consegnati[VACCINO_M]++
			} else {
				// azzero la quantità più piccola ed al resto ci pensa la quantità più grande
				if stato.dosi_disponibili[VACCINO_P] > stato.dosi_disponibili[VACCINO_M] {
					resto := NUM_DOSI_PRELIEVO - stato.dosi_disponibili[VACCINO_M]
					stato.dosi_disponibili[VACCINO_M] = 0
					stato.dosi_disponibili[VACCINO_P] -= resto
				} else {
					resto := NUM_DOSI_PRELIEVO - stato.dosi_disponibili[VACCINO_P]
					stato.dosi_disponibili[VACCINO_P] = 0
					stato.dosi_disponibili[VACCINO_M] -= resto
				}

				stato.lotti_consegnati[VACCINO_P]++
				stato.lotti_consegnati[VACCINO_M]++
			}

			fmt.Printf("[DEPOSITO] La regione: (%d; %s) ha fatto un prelievo\n", req.id, req.tipo)

		case req := <-when_dati_utente(cond_sinc_prelievo_rossa(stato), prelievo_chans[ROSSA]):
			if stato.fine {
				req.ack <- false
			} else {
				req.ack <- true
			}

			// aggiorno la quantità di lotti consegnati e le dosi di vaccino rimanenti
			if stato.dosi_disponibili[VACCINO_P] >= NUM_DOSI_PRELIEVO {
				stato.dosi_disponibili[VACCINO_P] -= NUM_DOSI_PRELIEVO
				stato.lotti_consegnati[VACCINO_P]++
			} else if stato.dosi_disponibili[VACCINO_M] >= NUM_DOSI_PRELIEVO {
				stato.dosi_disponibili[VACCINO_M] -= NUM_DOSI_PRELIEVO
				stato.lotti_consegnati[VACCINO_M]++
			} else {
				// azzero la quantità più piccola ed al resto ci pensa la quantità più grande
				if stato.dosi_disponibili[VACCINO_P] > stato.dosi_disponibili[VACCINO_M] {
					resto := NUM_DOSI_PRELIEVO - stato.dosi_disponibili[VACCINO_M]
					stato.dosi_disponibili[VACCINO_M] = 0
					stato.dosi_disponibili[VACCINO_P] -= resto
				} else {
					resto := NUM_DOSI_PRELIEVO - stato.dosi_disponibili[VACCINO_P]
					stato.dosi_disponibili[VACCINO_P] = 0
					stato.dosi_disponibili[VACCINO_M] -= resto
				}

				stato.lotti_consegnati[VACCINO_P]++
				stato.lotti_consegnati[VACCINO_M]++
			}

			fmt.Printf("[DEPOSITO] La regione: (%d; %s) ha fatto un prelievo\n", req.id, req.tipo)

		// canali case farmaceutiche
		case req := <-when_dati_utente(stato.fine || stato.dosi_disponibili[VACCINO_P]+NUM_DOSI_DEPOSITO <= CAPACITÀ_P, deposito_chans[VACCINO_P]):
			if stato.fine {
				req.ack <- false
			} else {
				req.ack <- true
			}

			stato.dosi_disponibili[VACCINO_P] += NUM_DOSI_DEPOSITO
			fmt.Printf("[DEPOSITO] La casa farmaceutica: (%d; %s) ha fatto un deposito\n", req.id, req.tipo)

		case req := <-when_dati_utente(stato.fine || stato.dosi_disponibili[VACCINO_M]+NUM_DOSI_DEPOSITO <= CAPACITÀ_M, deposito_chans[VACCINO_M]):
			if stato.fine {
				req.ack <- false
			} else {
				req.ack <- true
			}

			stato.dosi_disponibili[VACCINO_M] += NUM_DOSI_DEPOSITO
			fmt.Printf("[DEPOSITO] La casa farmaceutica: (%d; %s) ha fatto un deposito\n", req.id, req.tipo)

		// terminazione
		case <-termina_deposito:
			fmt.Println("FINE !!!!!!")
			done <- true
			return
		}

		// controllo se devo terminare gli utenti
		if stato.lotti_consegnati[VACCINO_P] >= MAX_CONSEGNE_P && stato.lotti_consegnati[VACCINO_M] >= MAX_CONSEGNE_M {
			stato.fine = true
		}

		// debug
		// stampa_stato(stato)
	}
}

func main() {
	rand.Seed(time.Now().Unix())

	//inizializzazione canali
	for i := 0; i < TIPI_ZONA; i++ {
		// asincroni perchè ho bisogno di usare len() per le politiche di priorità
		prelievo_chans[i] = make(chan dati_utente, 100)
	}
	for i := 0; i < TIPI_VACCINO; i++ {
		// asincroni perchè è comodo per la terminazione delle relative goroutine
		deposito_chans[i] = make(chan dati_utente)
	}

	go deposito()

	for i := 0; i < NUM_REGIONI; i++ {
		zona := rand.Intn(TIPI_ZONA)
		go regione(i, zona)
	}

	for i := 0; i < NUM_CASE_FARM; i++ {
		go casa_farm(i)
	}

	// attendo la fine degli utenti del deposito
	for i := 0; i < NUM_REGIONI+NUM_CASE_FARM; i++ {
		<-done
	}

	// termino il deposito ed il programma
	termina_deposito <- true
	<-done

	fmt.Printf("[main] APPLICAZIONE TERMINATA \n")
}
