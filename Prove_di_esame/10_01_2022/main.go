package main

import (
	"fmt"
	"math/rand"
	"time"
)

/*
COSTANTI DEL PROBLEMA
*/
const NUM_UTENTI = 4
const NUM_UFFICI = 1
const CAPACITÀ_SALA = 2

const TIPI_UTENTE = 3
const (
	PROPRIETARIO      = 0
	PROP_ACCOMPAGNATO = 1
	AMMINISTRATORE    = 2
)

var NOMI_UTENTE = [TIPI_UTENTE]string{"PROPRIETARIO", "PROP_ACCOMPAGNATO", "AMMINISTRATORE"}
var PERSONE_PER_UTENTE = [TIPI_UTENTE]int{1, 2, 1}

const TIPI_FINANZIAMENTO = 2
const (
	SUPERBONUS = 0
	ALTRO      = 1
)

var NOMI_FINANZIAMENTO = [TIPI_FINANZIAMENTO]string{"SUPERBONUS", "ALTRO"}

/*
STRUTTURE DATI
*/
type dati_utente struct {
	id          int
	tipo        string
	num_persone int
	ack         chan bool
}

type stato_gestore struct {
	persone_in_sala   int
	consulenti_liberi int
	fine              bool
}

/*
CANALI
*/
var entrata_sala_chans [TIPI_UTENTE]chan dati_utente
var consulenza_chans [TIPI_FINANZIAMENTO]chan dati_utente
var uscita chan dati_utente = make(chan dati_utente)

// canali usati per la terminazione delle goroutine
var done = make(chan bool)
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
	fmt.Printf("\tpersone in sala: %d\n", stato.persone_in_sala)
	fmt.Printf("\tconsulenti liberi: %d\n\n", stato.consulenti_liberi)
}

/*
GOROUTINE DEGLI UTENTI
*/
func utente(id int, tipo_utente int, tipo_finanziamento int) {
	req := dati_utente{id, NOMI_UTENTE[tipo_utente], PERSONE_PER_UTENTE[tipo_utente], make(chan bool)}
	sleepRandTime(3)
	fmt.Printf("[UTENTE (%d, %s)]: inizia!\n", id, NOMI_UTENTE[tipo_utente])

	entrata_sala_chans[tipo_utente] <- req
	<-req.ack
	fmt.Printf("[UTENTE (%d, %s)]: è entrato nella sala di attesa!\n", id, NOMI_UTENTE[tipo_utente])

	consulenza_chans[tipo_finanziamento] <- req
	<-req.ack
	fmt.Printf("[UTENTE (%d, %s)]: sta ricevendo la consulenza!\n", id, NOMI_UTENTE[tipo_utente])

	sleepRandTime(5) // tempo necessario per la consulenza

	fmt.Printf("[UTENTE (%d, %s)]: se ne va a casa!\n", id, NOMI_UTENTE[tipo_utente])
	uscita <- req

	done <- true
}

/*
CONDIZIONI DI SINCRONIZZAZIONE
*/
func cond_sinc_entrata_prop(stato stato_gestore, amm_in_coda int) bool {
	// condizioni di sincronizzazione
	if stato.persone_in_sala >= CAPACITÀ_SALA {
		// fmt.Printf("\tUtente non può entrare dato che non c'è posto in sala\n")
		return false
	}

	// regole di priorità
	if amm_in_coda > 0 {
		// fmt.Printf("\tUtente non può entrare dato che c'è un ammininastratore in coda")
		return false
	}

	return true
}

func cond_sinc_entrata_prop_accompagnato(stato stato_gestore, prio_in_coda int) bool {
	// condizioni di sincronizzazione
	if stato.persone_in_sala >= CAPACITÀ_SALA-1 {
		// fmt.Printf("\tUtente non può entrare dato che non c'è posto in sala\n")
		return false
	}

	// regole di priorità
	if prio_in_coda > 0 {
		// fmt.Printf("\tUtente non può entrare dato che c'è un ammininastratore/proprietario in coda")
		return false
	}

	return true
}

func cond_sinc_entrata_amministratore(stato stato_gestore) bool {
	// condizioni di sincronizzazione
	if stato.persone_in_sala >= CAPACITÀ_SALA {
		// fmt.Printf("\tUtente non può entrare dato che non c'è posto in sala\n")
		return false
	}

	// regole di priorità
	// l'amministratore ha priorità massima nell'entrare in sala

	return true
}

func cond_sinc_consulenza_superbonus(stato stato_gestore) bool {
	// condizioni di sincronizzazione
	if stato.consulenti_liberi == 0 {
		// fmt.Printf("\tUtente aspetta dato che non ci sono consulenti liberi\n")
		return false
	}

	// regole di priorità
	// superbonus ha priorità massima

	return true
}

func cond_sinc_consulenza_altro(stato stato_gestore, superbonus_in_coda int) bool {
	// condizioni di sincronizzazione
	if stato.consulenti_liberi == 0 {
		// fmt.Printf("\tUtente aspetta dato che non ci sono consulenti liberi\n")
		return false
	}

	// regole di priorità
	if superbonus_in_coda > 0 {
		// fmt.Printf("\tUtente aspetta dato che ci sono consulenza più prioritarie\n")
		return false
	}

	return true
}

/*
GOROUTINE GESTORE
*/
func filiale() {
	var stato stato_gestore
	stato.persone_in_sala = 0
	stato.consulenti_liberi = NUM_UFFICI
	stato.fine = false

	for {
		select {
		// canali per l'entrata
		case req := <-when_dati_utente(cond_sinc_entrata_prop(stato, len(entrata_sala_chans[AMMINISTRATORE])), entrata_sala_chans[PROPRIETARIO]):
			stato.persone_in_sala += req.num_persone
			req.ack <- true
			fmt.Printf("[FILIALE] L'utente: (%d; %s) è entrato in sala\n", req.id, req.tipo)

		case req := <-when_dati_utente(cond_sinc_entrata_prop_accompagnato(stato,
			len(entrata_sala_chans[AMMINISTRATORE])+len(entrata_sala_chans[PROPRIETARIO])), entrata_sala_chans[PROP_ACCOMPAGNATO]):

			stato.persone_in_sala += req.num_persone
			req.ack <- true
			fmt.Printf("[FILIALE] L'utente: (%d; %s) è entrato in sala\n", req.id, req.tipo)

		case req := <-when_dati_utente(cond_sinc_entrata_amministratore(stato), entrata_sala_chans[AMMINISTRATORE]):
			// uguale al caso del proprietario singolo, ma uso un canale distinto in modo da poter contare gli amministratori in coda
			stato.persone_in_sala += req.num_persone
			req.ack <- true
			fmt.Printf("[FILIALE] L'utente: (%d; %s) è entrato in sala\n", req.id, req.tipo)

		// canali per la consulenza
		case req := <-when_dati_utente(cond_sinc_consulenza_superbonus(stato), consulenza_chans[SUPERBONUS]):
			stato.persone_in_sala -= req.num_persone
			stato.consulenti_liberi--
			req.ack <- true
			fmt.Printf("[FILIALE] L'utente: (%d; %s) è entrato in un ufficio\n", req.id, req.tipo)

		case req := <-when_dati_utente(cond_sinc_consulenza_altro(stato, len(consulenza_chans[SUPERBONUS])), consulenza_chans[ALTRO]):
			// uguale al caso del superbonus, ma uso un canale distinto in modo da poter contare gli altri finanziamenti in coda
			stato.persone_in_sala -= req.num_persone
			stato.consulenti_liberi--
			req.ack <- true
			fmt.Printf("[FILIALE] L'utente: (%d; %s) è entrato in un ufficio\n", req.id, req.tipo)

		case req := <-uscita:
			stato.consulenti_liberi++
			fmt.Printf("[FILIALE] L'utente: (%d; %s), esce dalla filiale\n", req.id, req.tipo)

		// terminazione
		case <-termina:
			fmt.Println("FINE !!!!!!")
			done <- true
			return
		}

		// per debug:
		stampa_stato(stato)
	}
}

func main() {
	rand.Seed(time.Now().Unix())

	//inizializzazione canali
	for i := 0; i < TIPI_UTENTE; i++ {
		// asincrono perchè ho bisogno di usare len() per le politiche di priorità
		entrata_sala_chans[i] = make(chan dati_utente, 100)
	}
	for i := 0; i < TIPI_FINANZIAMENTO; i++ {
		// asincrono perchè ho bisogno di usare len() per le politiche di priorità
		consulenza_chans[i] = make(chan dati_utente, 100)
	}

	go filiale()

	for i := 0; i < NUM_UTENTI; i++ {
		tipo_utente := rand.Intn(TIPI_UTENTE)
		tipo_finanziamento := rand.Intn(TIPI_FINANZIAMENTO)
		go utente(i, tipo_utente, tipo_finanziamento)
	}

	// attendo la fine degli utenti
	for i := 0; i < NUM_UTENTI; i++ {
		<-done
	}

	// termino la filiale ed il programma
	termina <- true
	<-done

	fmt.Printf("[main] APPLICAZIONE TERMINATA \n")
}
