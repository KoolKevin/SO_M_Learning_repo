// pool di risorse equivalenti senza guardie logiche
package main

import (
	"fmt"
	"math/rand"
	"time"
)

const MAXPROC = 100 //massimo numero di processi
const MAX_BE = 5    //massimo numero di bici elettriche
const MAX_BT = 5

// ENUMERATIVO PER TIPOLOGIA DI BICI
type bici_type int

const (
	BICI_TRADIZIONALE bici_type = iota
	BICI_ELETTRICA
)

type bici_t struct {
	id_bici int
	tipo    bici_type
}

/* Canali */
var richiesta_tradizionale_chan = make(chan int)
var richiesta_elettrica_chan = make(chan int)
var richiesta_flex_chan = make(chan int)
var rilascio_chan = make(chan bici_t)
var bici_chan [MAXPROC]chan bici_t
var done_chan = make(chan int)
var termina_chan = make(chan int)

func client_tradizionale(req int) {
	fmt.Printf("\n[client %v] invia una richiesta tradizionale\n", req)
	richiesta_tradizionale_chan <- req
	bici := <-bici_chan[req]

	fmt.Printf("\n[client %v] uso della risorsa %v\n", req, bici)
	timeout := rand.Intn(3)
	time.Sleep(time.Duration(timeout) * time.Second)

	rilascio_chan <- bici
	done_chan <- req
}

func client_elettrico(req int) {
	fmt.Printf("\n[client %v] invia una richiesta elettrica\n", req)
	richiesta_elettrica_chan <- req
	bici := <-bici_chan[req]

	fmt.Printf("\n[client %v] uso della risorsa %v\n", req, bici)
	timeout := rand.Intn(3)
	time.Sleep(time.Duration(timeout) * time.Second)

	rilascio_chan <- bici
	done_chan <- req
}

func client_flex(req int) {
	fmt.Printf("\n[client %v] invia una richiesta flex\n", req)
	richiesta_flex_chan <- req
	bici := <-bici_chan[req]

	fmt.Printf("\n[client %v] uso della risorsa %v\n", req, bici)
	timeout := rand.Intn(3)
	time.Sleep(time.Duration(timeout) * time.Second)

	rilascio_chan <- bici
	done_chan <- req
}

func when(b bool, c chan int) chan int {
	if !b {
		fmt.Println("guardia logica fallita")
		return nil
	}
	return c
}

func server(num_bici_tradizionali int, num_bici_elettriche int) {
	//stato server
	var tradizionali_libere [MAX_BT]bool
	var elettriche_libere [MAX_BE]bool

	//variabili d'appoggio
	var req int
	var res, bici_rilasciata bici_t
	var i int
	disponibili_elettriche := num_bici_elettriche
	disponibili_tradizionali := num_bici_tradizionali

	/* inizializzazione stato server */
	for i := 0; i < num_bici_tradizionali; i++ {
		tradizionali_libere[i] = true
	}
	for i := 0; i < num_bici_elettriche; i++ {
		elettriche_libere[i] = true
	}

	for {
		time.Sleep(time.Second * 1)
		fmt.Println("\n\tNUOVO CICLO SERVER")

		fmt.Println("\nSTATO SERVER:",
			"\nRISORSE:", disponibili_tradizionali, ";", disponibili_elettriche)

		select {
		case req = <-when(disponibili_tradizionali > 0, richiesta_tradizionale_chan):
			//allocazione della risorsa
			for i = 0; i < num_bici_tradizionali && !tradizionali_libere[i]; i++ {
			}
			tradizionali_libere[i] = false
			disponibili_tradizionali--
			//costruzione e invio risposta
			res.id_bici = i
			res.tipo = BICI_TRADIZIONALE
			bici_chan[req] <- res
			fmt.Printf("[server]  allocata risorsa %v a cliente %v \n", res, req)

		case req = <-when(disponibili_elettriche > 0, richiesta_elettrica_chan):
			//allocazione della risorsa
			for i = 0; i < num_bici_elettriche && !elettriche_libere[i]; i++ {
			}
			elettriche_libere[i] = false
			disponibili_elettriche--
			//costruzione e invio risposta
			res.id_bici = i
			res.tipo = BICI_ELETTRICA
			bici_chan[req] <- res
			fmt.Printf("[server]  allocata risorsa %v a cliente %v \n", res, req)

		case req = <-when(disponibili_elettriche > 0 || disponibili_tradizionali > 0, richiesta_flex_chan):
			if disponibili_elettriche > 0 {
				//allocazione bici elettrica
				for i = 0; i < num_bici_elettriche && !elettriche_libere[i]; i++ {
				}
				elettriche_libere[i] = false
				disponibili_elettriche--
				//costruzione e invio risposta
				res.id_bici = i
				res.tipo = BICI_ELETTRICA
				bici_chan[req] <- res
				fmt.Printf("[server]  allocata risorsa %v a cliente %v \n", res, req)
			} else {
				//allocazione bici tradizionale
				for i = 0; i < num_bici_tradizionali && !tradizionali_libere[i]; i++ {
				}
				tradizionali_libere[i] = false
				disponibili_tradizionali--
				//costruzione e invio risposta
				res.id_bici = i
				res.tipo = BICI_TRADIZIONALE
				bici_chan[req] <- res
				fmt.Printf("[server]  allocata risorsa %v a cliente %v \n", res, req)
			}

		case bici_rilasciata = <-rilascio_chan:
			switch bici_rilasciata.tipo {
			case BICI_TRADIZIONALE:
				disponibili_tradizionali++
				tradizionali_libere[bici_rilasciata.id_bici] = true
				fmt.Printf("[server]  restituita risorsa: %v  \n", bici_rilasciata)

			case BICI_ELETTRICA:
				disponibili_elettriche++
				elettriche_libere[bici_rilasciata.id_bici] = true
				fmt.Printf("[server]  restituita risorsa: %v  \n", bici_rilasciata)
			}
		case <-termina_chan:
			// quando tutti i processi clienti hanno finito
			fmt.Println("FINE !!!!!!")
			done_chan <- 1

			return
		}
	}
}

func main() {
	var num_bici_elettriche, num_bici_tradizionali, num_richieste_tradizionali, num_richieste_elettriche, num_richieste_flex int

	rand.Seed(time.Now().Unix())
	fmt.Printf("\n quanti clienti tradizionali? ")
	fmt.Scanf("%d", &num_richieste_tradizionali)
	fmt.Printf("\n quanti clienti elettriche? ")
	fmt.Scanf("%d", &num_richieste_elettriche)
	fmt.Printf("\n quanti clienti flex? ")
	fmt.Scanf("%d", &num_richieste_flex)
	tot_richieste := num_richieste_tradizionali + num_richieste_elettriche + num_richieste_flex
	fmt.Println("clienti:", tot_richieste)

	fmt.Printf("\n quante tradizionali (max %d)? ", MAX_BT)
	fmt.Scanf("%d", &num_bici_tradizionali)
	fmt.Printf("\n quante elettriche (max %d)? ", MAX_BE)
	fmt.Scanf("%d", &num_bici_elettriche)
	fmt.Println("risorse da gestire:", num_bici_tradizionali, ";", num_bici_elettriche)

	//inizializzazione canali
	for i := 0; i < tot_richieste; i++ {
		bici_chan[i] = make(chan bici_t)
	}

	// lancio server
	go server(num_bici_tradizionali, num_bici_elettriche)

	//lancio clienti
	var i, j int
	for i = 0; i < num_richieste_tradizionali; i++ {
		go client_tradizionale(i)
	}
	for j = 0; j < num_richieste_elettriche; j++ {
		go client_elettrico(j + num_richieste_tradizionali)
	}
	// le richieste flex le considero come richieste elettriche speciali
	for k := 0; k < num_richieste_flex; k++ {
		go client_flex(k + num_richieste_tradizionali + num_richieste_elettriche)
	}

	//attesa della terminazione dei clienti:
	for i = 0; i < tot_richieste; i++ {
		<-done_chan
	}
	termina_chan <- 1 //terminazione server
	<-done_chan
}
