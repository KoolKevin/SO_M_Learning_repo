package main

import (
	"fmt"
	"math/rand"
	"time"
)

/*
COSTANTI DEL PROBLEMA
*/
const NUM_CLIENTI = 10
const NUM_COMMESSI = 2

const CAPACITÀ_NEGOZIO = 5
const CLIENTI_PER_COMMESSO = 2
const NUM_MASCHERINE = 5

const TIPI_UTENTE = 3
const (
	ABITUALE    = 0
	OCCASIONALE = 1
	COMMESSO    = 2
)

var NOMI_UTENTE = [TIPI_UTENTE]string{"CLIENTE ABITUALE", "CLIENTE OCCASIONALE", "COMMESSO"}

/*
STRUTTURE DATI
*/
type dati_req struct {
	id          int
	tipo        string
	ack         chan int // al cliente restituisco l'indice del commesso a cui è stato assegnato
	id_commesso int      // assegnato solo ai clienti
}

type stato_commesso struct {
	clienti_supervisionati int
	dentro                 bool
	vuole_uscire           bool
	ack                    chan int
}

type stato_gestore struct {
	persone_in_negozio int
	mascherine_rimaste int
	commessi           [NUM_COMMESSI]stato_commesso
	fine               bool
}

/*
CANALI
*/
var entrata_chans [TIPI_UTENTE]chan dati_req // per gestire le regole di priorità, distinguo in canali distinti i vari utenti che entrano
var uscita_clienti chan dati_req = make(chan dati_req)
var uscita_commessi chan dati_req = make(chan dati_req, 100)
var ritiro_mascherine chan dati_req = make(chan dati_req)
var consegna_mascherine chan bool = make(chan bool)
var continua_fornitore chan bool = make(chan bool)

// canali usati per la terminazione delle goroutine
var done = make(chan bool)
var termina_commessi = make(chan bool)
var termina = make(chan bool)

func when_bool(b bool, c chan bool) chan bool {
	if !b {
		return nil
	}
	return c
}

func when_dati_req(b bool, c chan dati_req) chan dati_req {
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

func stampa_commessi(commessi [NUM_COMMESSI]stato_commesso) {
	for i := 0; i < NUM_COMMESSI; i++ {
		fmt.Printf("\tcommesso: %d\n", i)
		fmt.Printf("\t\tclienti_supervisionati: %d\n", commessi[i].clienti_supervisionati)
		fmt.Printf("\t\tdentro: %v\n", commessi[i].dentro)
		fmt.Printf("\t\tvuole_uscire: %v\n", commessi[i].vuole_uscire)
	}
}

func stampa_stato(stato stato_gestore) {
	fmt.Printf("\tpersone_in_negozio: %d\n", stato.persone_in_negozio)
	fmt.Printf("\tmascherine_rimaste: %d\n", stato.mascherine_rimaste)
	stampa_commessi(stato.commessi)
	fmt.Println()

}

/*
GOROUTINE DEGLI UTENTI
*/
func commesso(id int) {
	req := dati_req{id, NOMI_UTENTE[COMMESSO], make(chan int), -1}

	sleepRandTime(3)
	fmt.Printf("[COMMESSO %d]: inizia a lavorare!\n", id)

	// I comessi lavorano continuamente finchè ci sono clienti
	for {
		fmt.Printf("[COMMESSO %d]: vuole entrare nel negozio!\n", id)
		entrata_chans[COMMESSO] <- req
		ack := <-req.ack
		if ack == -1 {
			fmt.Printf("[COMMESSO %d]: non ci sono più clienti, termino anche io!\n", id)
			done <- true
			return
		}

		sleepRandTime(5) // assistenza dei clienti

		fmt.Printf("[COMMESSO %d]: vuole uscire dal negozio!\n", id)
		uscita_commessi <- req
		<-req.ack

		fmt.Printf("\t[COMMESSO %d]: fa una pausa!\n", id)
		sleepRandTime(3) // pausa
	}
}

func cliente(id int, tipo int) {
	req := dati_req{id, NOMI_UTENTE[tipo], make(chan int), -1}

	fmt.Printf("[CLIENTE (%d, %s)] vuole ritirare la mascherina!\n", req.id, req.tipo)
	ritiro_mascherine <- req

	fmt.Printf("[CLIENTE (%d, %s)] vuole entrare nel negozio!\n", id, NOMI_UTENTE[tipo])
	entrata_chans[tipo] <- req
	req.id_commesso = <-req.ack

	sleepRandTime(3) // visita del negozio

	fmt.Printf("[CLIENTE (%d, %s)] vuole uscire dal negozio!\n", id, NOMI_UTENTE[tipo])
	uscita_clienti <- req
	fmt.Printf("[CLIENTE (%d, %s)]: torna a casa!\n", id, NOMI_UTENTE[tipo])

	done <- true
}

func fornitore() {
	sleepRandTime(3)
	fmt.Printf("[FORNITORE]: inizia a lavorare!\n")

	// Il fornitore lavora continuamente finchè ci sono clienti
	for {
		fmt.Printf("[FORNITORE] passa!\n")
		consegna_mascherine <- true
		continua := <-continua_fornitore
		if !continua {
			fmt.Printf("[FORNITORE]: non ci sono più clienti, termino anche io!\n")
			done <- true
			return
		}

		sleepRandTime(5) // tempo prima del prossimo rifornimento
	}
}

/*
CONDIZIONI DI SINCRONIZZAZIONE
(attenzione a condizioni con stato.fine)
*/

func cond_sinc_entrata_cliente_abituale(stato stato_gestore, commessi_in_coda int) bool {
	// condizioni di sincronizzazione
	if stato.persone_in_negozio >= CAPACITÀ_NEGOZIO {
		// non c'è abbastanza spazio nel negozio
		return false
	}

	indice_commesso := -1
	for i := 0; i < NUM_COMMESSI; i++ {
		if stato.commessi[i].dentro && stato.commessi[i].clienti_supervisionati < CLIENTI_PER_COMMESSO {
			indice_commesso = i
			break
		}
	}
	if indice_commesso == -1 {
		// non c'è un commesso libero per il cliente
		return false
	}

	// regole di priorità
	if commessi_in_coda > 0 {
		// ci sono già dei commessi che vogliono entrare
		return false
	}

	return true
}

func cond_sinc_entrata_cliente_occasionale(stato stato_gestore, più_prio_in_coda int) bool {
	// condizioni di sincronizzazione
	if stato.persone_in_negozio >= CAPACITÀ_NEGOZIO {
		// non c'è abbastanza spazio nel negozio
		return false
	}

	indice_commesso := -1
	for i := 0; i < NUM_COMMESSI; i++ {
		if stato.commessi[i].dentro && stato.commessi[i].clienti_supervisionati < CLIENTI_PER_COMMESSO {
			indice_commesso = i
			break
		}
	}
	if indice_commesso == -1 {
		// non c'è un commesso libero per il cliente
		return false
	}

	// regole di priorità
	if più_prio_in_coda > 0 {
		// ci sono già dei commessi/clienti abituali che vogliono entrare
		return false
	}

	return true
}

func cond_sinc_entrata_commesso(stato stato_gestore) bool {
	// condizioni di sincronizzazione
	if stato.persone_in_negozio >= CAPACITÀ_NEGOZIO {
		// non c'è abbastanza spazio nel negozio
		return false
	}

	// regole di priorità
	// il commesso ha priorità massima nel entrare nel negozio

	return true
}

/*
GOROUTINE GESTORE
*/
func negozio() {
	/*
		inizializzazione stato gestore
	*/
	var stato stato_gestore
	stato.persone_in_negozio = 0
	stato.mascherine_rimaste = NUM_MASCHERINE
	for i := 0; i < NUM_COMMESSI; i++ {
		stato.commessi[i].clienti_supervisionati = 0
		stato.commessi[i].dentro = false
		stato.commessi[i].vuole_uscire = false
	}
	stato.fine = false

	for {
		select {
		// canali clienti
		case req := <-when_dati_req(stato.mascherine_rimaste > 0, ritiro_mascherine):
			stato.mascherine_rimaste--
			fmt.Printf("[NEGOZIO] il client: (%d; %s) ha ritirato una mascherina\n", req.id, req.tipo)

		case req := <-when_dati_req(cond_sinc_entrata_cliente_abituale(stato, len(entrata_chans[COMMESSO])), entrata_chans[ABITUALE]):
			stato.persone_in_negozio++
			// cerco l'indice del primo commesso libero
			var indice_commesso int
			for i := 0; i < NUM_COMMESSI; i++ {
				if stato.commessi[i].dentro && stato.commessi[i].clienti_supervisionati < CLIENTI_PER_COMMESSO {
					indice_commesso = i
					break
				}
			}
			stato.commessi[indice_commesso].clienti_supervisionati++
			req.ack <- indice_commesso
			fmt.Printf("[NEGOZIO] il cliente (%d; %s): è entranto nel negozio supervisionato dal commesso %d\n", req.id, req.tipo, indice_commesso)

		// uguale a sopra per i clienti occasionali, distinguo per gestire la politica di priorità
		case req := <-when_dati_req(cond_sinc_entrata_cliente_occasionale(stato, len(entrata_chans[COMMESSO])+len(entrata_chans[ABITUALE])), entrata_chans[OCCASIONALE]):
			stato.persone_in_negozio++
			// cerco l'indice del primo commesso libero
			var indice_commesso int
			for i := 0; i < NUM_COMMESSI; i++ {
				if stato.commessi[i].dentro && stato.commessi[i].clienti_supervisionati < CLIENTI_PER_COMMESSO {
					indice_commesso = i
					break
				}
			}
			stato.commessi[indice_commesso].clienti_supervisionati++
			req.ack <- indice_commesso
			fmt.Printf("[NEGOZIO] il cliente (%d; %s): è entranto nel negozio supervisionato dal commesso %d\n", req.id, req.tipo, indice_commesso)

		case req := <-uscita_clienti:
			stato.persone_in_negozio--
			stato.commessi[req.id_commesso].clienti_supervisionati--
			fmt.Printf("[NEGOZIO] il cliente (%d; %s): è andato via\n", req.id, req.tipo)

			// controllo se c'è un commesso che stava aspettando di uscire
			for i := 0; i < NUM_COMMESSI; i++ {
				if stato.commessi[i].clienti_supervisionati == 0 && stato.commessi[i].vuole_uscire == true {
					stato.persone_in_negozio--
					stato.commessi[i].vuole_uscire = false
					stato.commessi[i].dentro = false
					stato.commessi[i].ack <- 1
					fmt.Printf("[NEGOZIO] il commesso (%d): è uscito dal negozio\n", i)
				}
			}

		// canali commessi
		case req := <-when_dati_req(cond_sinc_entrata_commesso(stato), entrata_chans[COMMESSO]):
			stato.persone_in_negozio++
			stato.commessi[req.id].dentro = true
			stato.commessi[req.id].clienti_supervisionati = 0
			if !stato.fine {
				req.ack <- 1
			} else {
				req.ack <- -1
			}

			fmt.Printf("[NEGOZIO] il commesso (%d): è entrato nel negozio\n", req.id)

		case req := <-uscita_commessi:
			if stato.commessi[req.id].clienti_supervisionati == 0 {
				// può uscire
				stato.persone_in_negozio--
				stato.commessi[req.id].dentro = false
				req.ack <- 1
				fmt.Printf("[NEGOZIO] il commesso (%d): è uscito dal negozio\n", req.id)
			} else {
				// deve aspettare che escano i suoi clienti
				stato.commessi[req.id].vuole_uscire = true
				stato.commessi[req.id].ack = req.ack
				fmt.Printf("[NEGOZIO] il commesso (%d): desidera uscire dal negozio\n", req.id)
			}
		// canali fornitore
		case <-consegna_mascherine:
			stato.mascherine_rimaste += NUM_MASCHERINE
			if !stato.fine {
				continua_fornitore <- true
			} else {
				continua_fornitore <- false
			}
			fmt.Printf("[NEGOZIO] il fornitore è passato\n")

		// terminazione
		case <-termina_commessi:
			stato.fine = true

		case <-termina:
			fmt.Println("FINE !!!!!!")
			done <- true
			return
		}

		// debug
		stampa_stato(stato)
	}
}

func main() {
	rand.Seed(time.Now().Unix())

	//inizializzazione canali
	for i := 0; i < TIPI_UTENTE; i++ {
		// asincroni perchè ho bisogno di usare len() per le politiche di priorità
		entrata_chans[i] = make(chan dati_req, 100)
	}

	go negozio()

	for i := 0; i < NUM_COMMESSI; i++ {
		go commesso(i)
	}

	go fornitore()

	for i := 0; i < NUM_CLIENTI; i++ {
		tipo := rand.Intn(TIPI_UTENTE - 1) // non considero i commessi
		go cliente(i, tipo)
	}

	// attendo la fine dei clienti
	for i := 0; i < NUM_CLIENTI; i++ {
		<-done
	}

	// termino i commessi (ed il fornitore) ed attendo la loro fine
	termina_commessi <- true
	for i := 0; i < NUM_COMMESSI+1; i++ {
		<-done
	}

	// termino il negozio ed il programma
	termina <- true
	<-done

	fmt.Printf("[main] APPLICAZIONE TERMINATA \n")
}
