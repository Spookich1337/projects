#define N 2     
#define P 2      
#define N_SQ 4    

byte A[N_SQ];
byte B[N_SQ];
byte C[N_SQ];

byte finished = 0;     
bool done_printing = false; 

proctype worker(byte id) {
    byte i = id;
    byte j, k, sum;
    byte idxA, idxB, idxC;

    do
    :: i < N ->
        j = 0;
        do
        :: j < N ->
            sum = 0;
            k = 0;
            do
            :: k < N ->
                idxA = i * N + k;
                idxB = k * N + j;
                sum = sum + A[idxA] * B[idxB];
                k = k + 1;
            :: k == N -> break;
            od;
            
            idxC = i * N + j;
            C[idxC] = sum; 
            j = j + 1;
        :: j == N -> break;
        od;
        i = i + P; 
    :: i >= N -> break;
    od;

    atomic { finished = finished + 1; }

    if
    :: id == 0 ->
        finished == P; 
        printf("Resulting Matrix C:\n");
        printf("[%d, %d]\n", C[0], C[1]);
        printf("[%d, %d]\n", C[2], C[3]);
        done_printing = true; 
    :: else -> skip;
    fi;
}

init {
    A[0] = 1; A[1] = 2;
    A[2] = 3; A[3] = 4;
    
    B[0] = 5; B[1] = 6;
    B[2] = 7; B[3] = 8; 

    atomic {
        run worker(0);
        run worker(1);
    }
}

/* Живучесть */
ltl liveness { <> (done_printing == true) }

/* Корректность */
ltl safety { [] (done_printing -> (C[0] == 19 && C[1] == 22 && C[2] == 43 && C[3] == 50)) }
