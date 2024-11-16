// pool di risorse equivalenti con guardie logiche

package main

import (
	"fmt"
	"time"
)

const MAXPROC = 100
const MAXRES = 5

var richiesta = make(chan int)
var rilascio = make(chan int)
var risorsa [MAXPROC]chan int
var done = make(chan int)
var termina = make(chan int)

func when(b bool, c chan int) chan int {
	if !b {
		return nil
	}
	return c
}

func client(i int) {
	richiesta <- i
	r := <-risorsa[i]

	fmt.Printf("\n [client %d] uso della risorsa %d\n", i, r)

	rilascio <- r
	done <- i
}

func server(nris int) {
	var disponibili int = nris
	var res, p, i int
	var libera [MAXRES]bool

	for i := 0; i < nris; i++ {
		libera[i] = true
	}

	for {
		time.Sleep(time.Second * 1)
		fmt.Println("nuovo ciclo server")

		select {
		case p = <-when(disponibili > 0, richiesta):
			for i = 0; i < nris && !libera[i]; i++ {
			}
			libera[i] = false
			disponibili--
			risorsa[p] <- i
			fmt.Printf("[server]  allocata risorsa %d a cliente %d \n", i, p)

		case res = <-rilascio:
			disponibili++
			libera[res] = true
			fmt.Printf("[server]  restituita risorsa: %d  \n", res)

		case <-termina: // quando tutti i processi clienti hanno finito
			fmt.Println("FINE !!!!!!")
			done <- 1
			return

		}
	}
}

func main() {
	var cli, res int
	fmt.Printf("\n quanti clienti (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &cli)
	fmt.Println("clienti:", cli)

	fmt.Printf("\n quante risorse (max %d)? ", MAXRES)
	fmt.Scanf("%d", &res)
	fmt.Println("risorse da gestire:", res)

	//inizializzazione canali
	for i := 0; i < MAXPROC; i++ {
		risorsa[i] = make(chan int)
	}

	for i := 0; i < cli; i++ {
		go client(i)
	}

	go server(res)

	for i := 0; i < cli; i++ {
		<-done
	}

	termina <- 1 //terminazione server
	<-done
}
