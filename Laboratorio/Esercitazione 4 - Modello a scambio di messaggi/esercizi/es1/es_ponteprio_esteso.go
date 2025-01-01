// ponte_sensounico_prio project main.go -- priorità di direzione ai veicoli da NORD
// ponte_senso_unico project main.go

package main

import (
	"fmt"
	"math/rand"
	"time"
)

const MAXBUFF = 100
const MAXPROC = 100

// capacità ponte
const MAX = 6

// costanti per tipologie di utente del ponte
const FATTORE_PEDONI_EQUIVALENTI_PER_AUTO = 5
const TIPOLOGIE_UTENTE int = 2
const PEDONE int = 0
const AUTO int = 1

// direzioni
const NORD int = 0
const SUD int = 1

var done = make(chan bool)
var termina = make(chan bool)
var entrata_N_chans [TIPOLOGIE_UTENTE]chan int
var entrata_S_chans [TIPOLOGIE_UTENTE]chan int
var uscita_N_chans [TIPOLOGIE_UTENTE]chan int
var uscita_S_chans [TIPOLOGIE_UTENTE]chan int
var ACK_N_chans [TIPOLOGIE_UTENTE][MAXPROC]chan int //risposte client nord
var ACK_S_chans [TIPOLOGIE_UTENTE][MAXPROC]chan int //risposte client sud

func when(b bool, c chan int) chan int {
	if !b {
		return nil
	}
	return c
}

func utente(myid int, tipo int, dir int) {
	var tt int
	tt = rand.Intn(5) + 1
	fmt.Printf("inizializzazione utente %d direzione %d in secondi %d \n", myid, dir, tt)
	time.Sleep(time.Duration(tt) * time.Second)

	if dir == NORD {
		entrata_N_chans[tipo] <- myid // send asincrona
		<-ACK_N_chans[tipo][myid]     // attesa x sincronizzazione
		fmt.Printf("[utente %d; tipo %d]  entrato sul ponte in direzione  \tNORD\n", myid, tipo)

		tt = rand.Intn(5)
		time.Sleep(time.Duration(tt) * time.Second)

		uscita_N_chans[tipo] <- myid
		fmt.Printf("[utente %d; tipo %d]  uscito dal ponte in direzione  \tNORD\n", myid, tipo)
	} else {
		entrata_S_chans[tipo] <- myid
		<-ACK_S_chans[tipo][myid] // attesa x sincronizzazione
		fmt.Printf("[utente %d; tipo %d]  entrato sul ponte in direzione  \tSUD\n", myid, tipo)

		tt = rand.Intn(5)
		time.Sleep(time.Duration(tt) * time.Second)

		uscita_S_chans[tipo] <- myid
		fmt.Printf("[utente %d; tipo %d]  uscito dal ponte in direzione  \tSUD\n", myid, tipo)
	}

	done <- true
}

func server() {
	var cont_pedoni_N int = 0
	var cont_pedoni_S int = 0
	var cont_auto_N int = 0
	var cont_auto_S int = 0

	for {
		pedoni_equivalenti_sul_ponte := (cont_pedoni_N + cont_pedoni_S) + (cont_auto_N+cont_auto_S)*FATTORE_PEDONI_EQUIVALENTI_PER_AUTO

		select {
		// canali pedoni
		case x := <-when(cont_auto_N == 0 && (pedoni_equivalenti_sul_ponte+1 < MAX), entrata_S_chans[PEDONE]):
			fmt.Printf("\tSONO UN PEDONE\tstato ponte prima di entrate: PEDONI (%d, %d)\t AUTO(%d, %d)\n", cont_pedoni_S, cont_pedoni_N, cont_auto_S, cont_auto_N)
			cont_pedoni_S++
			ACK_S_chans[PEDONE][x] <- 1

		case x := <-when(cont_auto_S == 0 && (pedoni_equivalenti_sul_ponte+1 < MAX) && (len(entrata_S_chans[PEDONE]) == 0), entrata_N_chans[PEDONE]):
			fmt.Printf("\tSONO UN PEDONE\tstato ponte prima di entrate: PEDONI (%d, %d)\t AUTO(%d, %d)\n", cont_pedoni_S, cont_pedoni_N, cont_auto_S, cont_auto_N)
			cont_pedoni_N++
			ACK_N_chans[PEDONE][x] <- 1

		case <-uscita_S_chans[PEDONE]:
			fmt.Printf("\tSONO UN PEDONE\t esco dalla direzione SUD\n")
			cont_pedoni_S--

		case <-uscita_N_chans[PEDONE]:
			fmt.Printf("\tSONO UN PEDONE\t esco dalla direzione NORD\n")
			cont_pedoni_N--

		// canali auto
		case x := <-when(((1*FATTORE_PEDONI_EQUIVALENTI_PER_AUTO+pedoni_equivalenti_sul_ponte) < MAX) &&
			(cont_auto_N == 0) &&
			(len(entrata_S_chans[PEDONE]) == 0 && len(entrata_N_chans[PEDONE]) == 0),
			entrata_S_chans[AUTO]):

			fmt.Printf("\tSONO UNA AUTO\tstato ponte prima di entrate: PEDONI (%d, %d)\t AUTO(%d, %d)\n", cont_pedoni_S, cont_pedoni_N, cont_auto_S, cont_auto_N)
			cont_auto_S++
			ACK_S_chans[AUTO][x] <- 1

		case x := <-when(((1*FATTORE_PEDONI_EQUIVALENTI_PER_AUTO+pedoni_equivalenti_sul_ponte) < MAX) &&
			(cont_auto_S == 0) &&
			(len(entrata_S_chans[PEDONE]) == 0 && len(entrata_N_chans[PEDONE]) == 0) &&
			(len(entrata_S_chans[AUTO]) == 0),
			entrata_N_chans[AUTO]):

			fmt.Printf("\tSONO UNA AUTO\tstato ponte prima di entrate: PEDONI (%d, %d)\t AUTO(%d, %d)\n", cont_pedoni_S, cont_pedoni_N, cont_auto_S, cont_auto_N)
			cont_auto_N++
			ACK_N_chans[AUTO][x] <- 1

		case <-uscita_N_chans[AUTO]:
			fmt.Printf("\tSONO UN AUTO\t esco dalla direzione NORD\n")
			cont_auto_N--

		case <-uscita_S_chans[AUTO]:
			fmt.Printf("\tSONO UN AUTO\t esco dalla direzione SUD\n")
			cont_auto_S--

		case <-termina: // quando tutti i processi hanno finito
			fmt.Println("FINE !!!!!!")
			done <- true
			return
		}
	}
}

func main() {
	var PN int
	var PS int
	var VN int
	var VS int

	fmt.Printf("\n quanti pedoni NORD (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &PN)
	fmt.Printf("\n quanti pedoni SUD (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &PS)
	fmt.Printf("\n quanti veicoli NORD (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &VN)
	fmt.Printf("\n quanti veicoli SUD (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &VS)

	//inizializzazione canali
	for j := 0; j < TIPOLOGIE_UTENTE; j++ {
		entrata_S_chans[j] = make(chan int, MAXBUFF)
		uscita_S_chans[j] = make(chan int)

		for i := 0; i < VS; i++ {
			ACK_S_chans[j][i] = make(chan int, MAXBUFF)
		}
	}

	for j := 0; j < TIPOLOGIE_UTENTE; j++ {
		entrata_N_chans[j] = make(chan int, MAXBUFF)
		uscita_N_chans[j] = make(chan int)

		for i := 0; i < VN; i++ {
			ACK_N_chans[j][i] = make(chan int, MAXBUFF)
		}
	}

	rand.Seed(time.Now().Unix())

	// lancio goroutine
	go server()
	for i := 0; i < PS; i++ {
		go utente(i, PEDONE, SUD)
	}
	for i := 0; i < PN; i++ {
		go utente(i, PEDONE, NORD)
	}
	for i := 0; i < VS; i++ {
		go utente(i, AUTO, SUD)
	}
	for i := 0; i < VN; i++ {
		go utente(i, AUTO, NORD)
	}
	// aspetto che tutti abbiano terminato
	for i := 0; i < PN+PS+VN+VS; i++ {
		<-done
	}
	termina <- true
	<-done
	fmt.Printf("\n HO FINITO ")
}
