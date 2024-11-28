/*
	- 2 modelli di auto (modelloA e modelloB)
	- 4 nastri trasportatori (1 per ogni tipologia di cerchio e pneumatico)
	- Il deposito è caratterizzato da capacità:
		- MaxP
		- MaxC
	- montaggio con due bracci (uno per modello)
	- Per il montaggio delle ruote di un’auto*, ognuno dei due robot procede quindi come segue. Per ognuna delle 4 ruote:
		• preleva un cerchio e lo monta;
		• preleva uno pneumatico e lo monta.

	Il deposito dovrà gestire gli accessi da parte dei nastri e dei robot in modo da soddisfare i vincoli dati,
	ed inoltre i vincoli di priorità.
*/

package main

import (
	"fmt"
	"math/rand"
	"time"
)

// Numero auto da costruire
// TOT*4 cerchioni e pneumatici da produrre
// TOT*4 montaggi da fare
const TOT = 2
const MAX_P = 10
const MAX_C = 10

// costanti enumerative
const NUM_NASTRI = 4
const (
	NASTRO_CA = iota
	NASTRO_CB
	NASTRO_PA
	NASTRO_PB
)

const NUM_ROBOT = 2
const (
	ROBOT_A = iota
	ROBOT_B
)

// Distinguo direttamente i canali così mi evito uno switchone
// nel processo deposito.
// Il contenuto dei messaggi qua è quindi irrilevante
var nastro_CA_chan = make(chan bool)
var nastro_CB_chan = make(chan bool)
var nastro_PA_chan = make(chan bool)
var nastro_PB_chan = make(chan bool)

var robot_A_request_chan = make(chan bool)
var robot_B_request_chan = make(chan bool)
var robot_CA_chan = make(chan bool)
var robot_CB_chan = make(chan bool)
var robot_PA_chan = make(chan bool)
var robot_PB_chan = make(chan bool)

var done = make(chan bool)
var termina_deposito = make(chan bool)

// canale particolare
var continua_robot_chan = make(chan bool, 100)

func when(b bool, c chan bool) chan bool {
	if !b {
		return nil
	}
	return c
}

func nastro(tipologia_nastro int) {
	//ogni nastro devo produrre pnemuatici/cerchioni per 4 ruote di TOT auto
	for i := 0; i < TOT*4; i++ {
		// tempo di consegna del nastro
		tt := rand.Intn(1) + 1
		time.Sleep(time.Duration(tt) * time.Second)

		switch tipologia_nastro {
		case NASTRO_CA:
			nastro_CA_chan <- true
		case NASTRO_CB:
			nastro_CB_chan <- true
		case NASTRO_PA:
			nastro_PA_chan <- true
		case NASTRO_PB:
			nastro_PB_chan <- true
		}
	}

	// done <- true
}

func robot(tipologia_robot int) {
	//ogni nastro devo montare pnemuatici/cerchioni per TOT auto
	for continua := range continua_robot_chan {
		if !continua {
			fmt.Println("[ROBOT] mi interrompo")
			break
		}

		switch tipologia_robot {
		case ROBOT_A:
			robot_A_request_chan <- true
			<-robot_CA_chan
			<-robot_PA_chan
			fmt.Println("[ROBOT A] montata nuova ruota")
		case ROBOT_B:
			robot_B_request_chan <- true
			<-robot_CB_chan
			<-robot_PB_chan
			fmt.Println("[ROBOT B] montata nuova ruota")
		}
	}

	fmt.Println("[ROBOT] Non ho più niente da montare")

	done <- true
}

func deposito() {
	cont_ruote_montate_tipo_A := 0
	cont_ruote_montate_tipo_B := 0
	cont_cerchioni_tipo_A := 0
	cont_cerchioni_tipo_B := 0
	cont_pneumatici_tipo_A := 0
	cont_pneumatici_tipo_B := 0

	conta_per_stampa_tipo_A := 0
	conta_per_stampa_tipo_B := 0

	// attiva i robot
	for i := 0; i < NUM_ROBOT; i++ {
		continua_robot_chan <- true
	}

	for {
		// recupero il modello con meno ruote montate
		flag_priorità_A := cont_ruote_montate_tipo_A < cont_ruote_montate_tipo_B
		// recupero il numero di auto finite
		var auto_finite int = cont_ruote_montate_tipo_A/4 + cont_ruote_montate_tipo_B/4

		select {
		// canali dei nastri
		// Nell’immissione venga data la precedenza a pneumatici e cerchi del tipo corrispondente al modello di auto con il
		// minor numero di montaggi ruote completati;
		case <-when((cont_cerchioni_tipo_A < MAX_C) && (flag_priorità_A), nastro_CA_chan):
			fmt.Println("\t[NASTRO CERCHIONI TIPO A] nuovo pezzo consegnato")
			cont_cerchioni_tipo_A++

		case <-when((cont_cerchioni_tipo_B < MAX_C) && (!flag_priorità_A), nastro_CB_chan):
			fmt.Println("\t[NASTRO CERCHIONI TIPO B] nuovo pezzo consegnato")
			cont_cerchioni_tipo_B++

		case <-when((cont_pneumatici_tipo_A < MAX_P) && (flag_priorità_A), nastro_PA_chan):
			fmt.Println("\t[NASTRO PNEUMATICI TIPO A] nuovo pezzo consegnato")
			cont_pneumatici_tipo_A++

		case <-when((cont_pneumatici_tipo_B < MAX_P) && (!flag_priorità_A), nastro_PB_chan):
			fmt.Println("\t[NASTRO PNEUMATICI TIPO B] nuovo pezzo consegnato")
			cont_pneumatici_tipo_B++

		// canali delle richieste dei robot
		// Anche qua stessa regola di priorità
		case <-when((cont_pneumatici_tipo_A > 0 && cont_cerchioni_tipo_A > 0) && (flag_priorità_A), robot_A_request_chan):
			cont_ruote_montate_tipo_A++
			cont_cerchioni_tipo_A--
			cont_pneumatici_tipo_A--
			robot_CA_chan <- true
			robot_PA_chan <- true

			conta_per_stampa_tipo_A++

			// controllo se devo fare andare avanti i robot o meno
			if auto_finite < TOT {
				for i := 0; i < NUM_ROBOT; i++ {
					continua_robot_chan <- true
				}
			} else {
				for i := 0; i < NUM_ROBOT; i++ {
					continua_robot_chan <- false
				}

				close(continua_robot_chan)
			}

			// stampa
			if conta_per_stampa_tipo_A == 4 {
				fmt.Printf("\n------- FINITA AUTO NUMERO %d -------\n\n", auto_finite)
				conta_per_stampa_tipo_A = 0
			}

		case <-when((cont_pneumatici_tipo_B > 0 && cont_cerchioni_tipo_B > 0) && (!flag_priorità_A), robot_B_request_chan):
			cont_ruote_montate_tipo_B++
			cont_cerchioni_tipo_B--
			cont_pneumatici_tipo_B--
			robot_CB_chan <- true
			robot_PB_chan <- true

			conta_per_stampa_tipo_B++

			// controllo se devo fare andare avanti i robot o meno
			if auto_finite < TOT {
				for i := 0; i < NUM_ROBOT; i++ {
					continua_robot_chan <- true
				}
			} else {
				for i := 0; i < NUM_ROBOT; i++ {
					continua_robot_chan <- false
				}

				close(continua_robot_chan)
			}

			// stampa
			if conta_per_stampa_tipo_B == 4 {
				fmt.Println("------- FINITA AUTO NUMERO", auto_finite, "---------")
				conta_per_stampa_tipo_B = 0
			}

		// quando tutti i processi hanno finito
		case <-termina_deposito:
			fmt.Println("FINE !!!!!!")
			done <- true
			return
		}
	}
}

func main() {
	rand.Seed(time.Now().Unix())

	// lancio goroutine
	go deposito()
	for i := 0; i < NUM_NASTRI; i++ {
		go nastro(i)
	}

	for i := 0; i < NUM_ROBOT; i++ {
		go robot(i)
	}

	// aspetto che tutti abbiano terminato
	for i := 0; i < NUM_ROBOT; i++ {
		<-done
	}
	termina_deposito <- true
	<-done
	fmt.Printf("\n HO FINITO ")
}
