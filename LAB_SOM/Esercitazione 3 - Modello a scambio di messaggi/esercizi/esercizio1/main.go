// pool di risorse equivalenti senza guardie logiche
package main

import (
	"fmt"
	"math/rand"
	"time"
)

const MAXPROC = 100 //massimo numero di processi
const MAX_BE = 2    //massimo numero di bici elettriche
const MAX_BT = 3

// ENUMERATIVO PER RICHIESTE
type req_type int

const (
	REQ_TRADIZIONALE req_type = iota
	REQ_ELETTRICA
	REQ_FLEX
)

type req_t struct {
	id_cliente int
	tipo       req_type
}

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
var richiesta_chan = make(chan req_t)
var rilascio_chan = make(chan bici_t)
var bici_chan [MAXPROC]chan bici_t
var done_chan = make(chan int)
var termina_chan = make(chan int)

func client(req req_t) {
	richiesta_chan <- req
	bici := <-bici_chan[req.id_cliente]

	fmt.Printf("\n[client %v] uso della risorsa %v\n", req, bici)
	timeout := rand.Intn(3)
	time.Sleep(time.Duration(timeout) * time.Second)

	rilascio_chan <- bici
	done_chan <- req.id_cliente
}

func server(num_bici_tradizionali int, num_bici_elettriche int, num_richieste_tradizionali int, num_richieste_elettriche int) {
	//stato server
	var tradizionali_libere [MAX_BT]bool
	var elettriche_libere [MAX_BE]bool
	var sospesi_tradizionali [MAXPROC]bool
	var sospesi_elettriche [MAXPROC]bool
	var nsosp_tradizionali int = 0
	var nsosp_elettriche int = 0
	//variabili d'appoggio
	var req req_t
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
	for i := 0; i < num_richieste_tradizionali; i++ {
		sospesi_tradizionali[i] = false
	}
	for i := 0; i < num_richieste_elettriche; i++ {
		sospesi_elettriche[i] = false
	}

	for {
		time.Sleep(time.Second * 1)
		fmt.Println("\n\tNUOVO CICLO SERVER")

		fmt.Println("\nSTATO SERVER:",
			"\nRISORSE:", disponibili_tradizionali, ";", disponibili_elettriche,
			"\nSOSPESI:", nsosp_tradizionali, "; ", nsosp_elettriche)
		select {
		case req = <-richiesta_chan:
			switch req.tipo {
			case REQ_TRADIZIONALE:
				if disponibili_tradizionali > 0 {
					//allocazione della risorsa
					for i = 0; i < num_bici_tradizionali && !tradizionali_libere[i]; i++ {
					}
					tradizionali_libere[i] = false
					disponibili_tradizionali--
					//costruzione e invio risposta
					res.id_bici = i
					res.tipo = BICI_TRADIZIONALE
					bici_chan[req.id_cliente] <- res
					fmt.Printf("[server]  allocata risorsa %v a cliente %v \n", res, req)

				} else {
					// attesa
					nsosp_tradizionali++
					sospesi_tradizionali[req.id_cliente] = true
					fmt.Printf("[server]  il cliente %v attende..\n", req)
				}
			case REQ_ELETTRICA:
				if disponibili_elettriche > 0 {
					//allocazione della risorsa
					for i = 0; i < num_bici_elettriche && !elettriche_libere[i]; i++ {
					}
					elettriche_libere[i] = false
					disponibili_elettriche--
					//costruzione e invio risposta
					res.id_bici = i
					res.tipo = BICI_ELETTRICA
					bici_chan[req.id_cliente] <- res
					fmt.Printf("[server]  allocata risorsa %v a cliente %v \n", res, req)
				} else {
					// attesa
					nsosp_elettriche++
					sospesi_elettriche[req.id_cliente] = true
					fmt.Printf("[server]  il cliente %v attende..\n", req)
				}
			case REQ_FLEX:
				/*
					1. se c’è un’e-bike disponibile, il cliente ottiene l’e-bike.
					2. se non ci sono e-bike disponibili, e ce n’è almeno una tradizionale, il cliente ottiene una bici tradizionale;
					3. se non ci sono bici disponibili (di nessun tipo), il cliente attende un’e-bike.
				*/

				if disponibili_elettriche > 0 {
					//allocazione della risorsa
					for i = 0; i < num_bici_tradizionali && !elettriche_libere[i]; i++ {
					}
					elettriche_libere[i] = false
					disponibili_elettriche--
					//costruzione e invio risposta
					res.id_bici = i
					res.tipo = BICI_ELETTRICA
					bici_chan[req.id_cliente] <- res
					fmt.Printf("[server]  allocata risorsa %v a cliente %v \n", res, req)
				} else if disponibili_tradizionali > 0 {
					//allocazione della risorsa
					for i = 0; i < num_bici_tradizionali && !tradizionali_libere[i]; i++ {
					}
					tradizionali_libere[i] = false
					disponibili_tradizionali--
					//costruzione e invio risposta
					res.id_bici = i
					res.tipo = BICI_TRADIZIONALE
					bici_chan[req.id_cliente] <- res
					fmt.Printf("[server]  allocata risorsa %v a cliente %v \n", res, req)
				} else {
					// attesa
					nsosp_elettriche++
					sospesi_elettriche[req.id_cliente] = true
					fmt.Printf("[server]  il cliente %v attende..\n", req)
				}
			default:
				fmt.Printf("non-zero")
			}
		case bici_rilasciata = <-rilascio_chan:
			switch bici_rilasciata.tipo {
			case BICI_TRADIZIONALE:
				if nsosp_tradizionali == 0 {
					disponibili_tradizionali++
					tradizionali_libere[bici_rilasciata.id_bici] = true
					fmt.Printf("[server]  restituita risorsa: %v  \n", bici_rilasciata)
				} else {
					//risveglio il primo cliente sospeso che trovo
					for i = 0; i < num_richieste_tradizionali && !sospesi_tradizionali[i]; i++ {
					}
					sospesi_tradizionali[i] = false
					nsosp_tradizionali--
					fmt.Printf("[server]  trasferita risorsa: %v  \n", bici_rilasciata)
					bici_chan[i] <- bici_rilasciata
				}
			case BICI_ELETTRICA:
				if nsosp_elettriche == 0 {
					disponibili_elettriche++
					elettriche_libere[bici_rilasciata.id_bici] = true
					fmt.Printf("[server]  restituita risorsa: %v  \n", bici_rilasciata)
				} else {
					//risveglio il primo client che trovo
					for i = 0; i < num_richieste_elettriche && !sospesi_elettriche[i]; i++ {
					}
					sospesi_elettriche[i] = false
					nsosp_elettriche--
					fmt.Printf("[server]  trasferita risorsa: %v  \n", bici_rilasciata)
					bici_chan[i] <- bici_rilasciata
				}
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
	go server(num_bici_tradizionali, num_bici_elettriche, num_richieste_tradizionali, num_richieste_elettriche+num_richieste_flex)

	//lancio clienti

	/*
		OCCHIO!
		qua ho fatto un casino con gli indici ho separato i vettori dei sospesi tra tradizionali e elettrici,
		ma non ho separato il vettore dei canali di ricezione delle bici. Di conseguenza i clienti che aspettano una bici tradizionale
		e i clienti che aspettano una bici elettrica hanno una sovrapposizione nel vettore dei canali :(

		Possibile assegnamento di una bici di tipo sbagliato...
	*/
	var i int
	var req req_t
	for i = 0; i < num_richieste_tradizionali; i++ {
		req.id_cliente = i
		req.tipo = REQ_TRADIZIONALE
		go client(req)
	}
	for i = 0; i < num_richieste_elettriche; i++ {
		req.id_cliente = i
		req.tipo = REQ_ELETTRICA
		go client(req)
	}
	// le richieste flex le considero come richieste elettriche speciali
	for j := i; j < num_richieste_elettriche+num_richieste_flex; j++ {
		req.id_cliente = j
		req.tipo = REQ_FLEX
		go client(req)
	}

	//attesa della terminazione dei clienti:
	for i = 0; i < tot_richieste; i++ {
		<-done_chan
	}
	termina_chan <- 1 //terminazione server
	<-done_chan
}
