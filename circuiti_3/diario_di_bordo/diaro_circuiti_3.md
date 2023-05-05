# Parte 1
- Si costruisce il circuito RC come da figura sulla scheda di lab
#### Prima facciamo RC
utilizziamo le seguenti componenti per il circuito:
- C = 47nF ($\pm 1$)
- R = 9.9 $k\Omega$ ($\pm 0.1$)
Colleghiamo la sonda del CH1 davanti al condesatore e quella del CH2 subito dopo il condesatore:
[[parte_1_rc_circuito.jpg]]

Esploriamo diverse frequenze da circa 50Hz a 3000Hz con incertezza 10Hz

Printiamo i valori dall'oscilloscopio e troviamo differenza di fase:
Ora stiamo usando la trasformata di hilbert ma *probabilmente è meglio fittare con sin(wt + phi) e fare la differenza tra le phi.*

#### Poi facciamo RL
[[parte_1_rl_circuito.jpg]]
Esploriamo frequenze da 10kHz a 80kHz con incertezza 100Hz

Nota: per l'induttanza non è necessario misurarne il valore (ha detto il prof) ma abbiamo comunque preso il plot dell'induttanza (V-t) con onda quadra shiftata in modo che andasse da 0V a 2.2V con frequenza = 18370Hz.

### RLC
montiamo il circuito in modo da avere il componente che si vuole andare a misurare di volta in volta come ultimo.
**COMPONENTI:**
- R =  9.9 $k\Omega$ ($\pm 0.1$)
- C = 47nF ($\pm 1$)
-  L = ... (da trovare se serve)


### Note prof
-  per RLC il componente da misurare dev'essere sempre in fondo
- Si potra poi forse confrontare RL e RLC per vedere la L?