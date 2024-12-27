package main

import (
	"fmt"
	"math/rand"
	"time"
)

const CITTADINI_CON_RICHIESTE_PICCOLE = 5
const CITTADINI_CON_RICHIESTE_GRANDI = 5
const CAPACITÀ_SERBATOIO = 6.0
const CAPACITÀ_CASSETTA_10 = 1
const CAPACITÀ_CASSETTA_20 = 3

var tipi_richiesta = [2]string{"piccola", "grande"}

type cittadino_req_t struct {
	id_cittadino int
	tipo         string
}

/*
	NB: i canali utilizzati per mandare messaggi che hanno bisogno di
	una gestione particolare della priorità sono ASINCRONI.

	In questo modo posso utilizzare 'len()' per vedere se c'è qualcuno di
	più prioritario che sta aspettando.

	Al contrario i canali che non hanno bisogno di priorità sono sincroni
*/

// canali usati dai cittadini per richiedere un prelievo d'acqua
var prelievo_piccolo = make(chan int, 100)
var prelievo_grande = make(chan int, 100)

// canali usati dall'addetto per richiedere, ed avvisare della fine,
// del suo intervento
var req_intervento = make(chan int, 100)
var fine_intervento = make(chan int)

// canali usati dall'erogatore rispondere alle richieste
var eroga_piccolo = make(chan int)
var eroga_grande = make(chan int)
var ok_intervento = make(chan int)

var done = make(chan bool)
var terminaDeposito = make(chan bool)

func when(b bool, c chan int) chan int {
	if !b {
		return nil
	}
	return c
}

func addetto() {
	fmt.Printf("[Adetto]: inizia a lavorare!\n")
	var durata_intervento int = 1
	var tempo_tra_interventi int

	// l'addetto lavora continuamente finche qualcuno non gli dice di
	for {
		req_intervento <- 1 // valore arbitraio non importante
		<-ok_intervento
		time.Sleep(time.Duration(durata_intervento) * time.Second)
		fine_intervento <- 1 // valore arbitraio non importante

		tempo_tra_interventi = (rand.Intn(15) + 1)
		time.Sleep(time.Duration(tempo_tra_interventi) * time.Second)
	}
}

func cittadino(cittadino_req cittadino_req_t) {
	fmt.Printf("[cittadino %d] vuole fare un prelievo %s!\n", cittadino_req.id_cittadino, cittadino_req.tipo)

	switch {
	case cittadino_req.tipo == tipi_richiesta[0]: //piccola
		prelievo_piccolo <- cittadino_req.id_cittadino
		<-eroga_piccolo
		fmt.Printf("[cittadino %d] ha fatto un prelievo PICCOLO\n", cittadino_req.id_cittadino)

	case cittadino_req.tipo == tipi_richiesta[1]: //grande
		prelievo_grande <- cittadino_req.id_cittadino
		<-eroga_grande
		fmt.Printf("[cittadino %d] ha fatto un prelievo GRANDE\n", cittadino_req.id_cittadino)

	default:
		fmt.Printf("[cittadino %d]: ha fatto una richiesta strana!\n", cittadino_req.id_cittadino)
	}
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

func erogatore() {
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

	<-done

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
