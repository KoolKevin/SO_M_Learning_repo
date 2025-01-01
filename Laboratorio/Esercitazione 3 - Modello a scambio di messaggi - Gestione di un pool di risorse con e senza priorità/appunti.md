....
carenza del go è che nella istruzioni di comando con guardia __manca la possibilità di specificare una guardia logica__

Conseguenze:
- manca il caso di guardia fallita
    - c'è solo il caso di guardia valida e di guardia ritardata

possiamo però costruirci le guardie logiche a mano, tramite una funzione che restituisce un canale.

formalmente continua però a non esserci il caso di guardia fallita. Questo spiega come mai predisponiamo un ramo apposito per uscire dal comando con guardia