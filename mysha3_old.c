#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define SET_BIT(x, bit) (x |= ((unsigned long long)1 << bit))
#define UNSET_BIT(x, bit) (x &= ~((unsigned long long)1 << bit))
#define GET_BIT(x, bit) ((x & ((unsigned long long)1 << bit)) >> bit)

unsigned char digest[1024];
unsigned long long A[5][5];

unsigned long long left_shift(unsigned long long x, int shift){
    return (x << shift) | (x >> (64 - shift));
}

void theta(){
    unsigned long long C[5], D[5];

    memset(C, 0, sizeof(C));
    memset(D, 0, sizeof(D));

    for(int x = 0;x < 5;x ++){
        C[x] = A[x][0] ^ A[x][1] ^ A[x][2] ^ A[x][3] ^ A[x][4];
    }

    for(int x = 0;x < 5;x ++){
        D[x] = C[(((x - 1) % 5) + 5 ) % 5] ^ left_shift(C[(x + 1) % 5], 1);
    }

    for(int x = 0;x < 5;x ++){
        for(int y = 0;y < 5;y ++){
            A[x][y] = A[x][y] ^ D[x];
        }
    }
}

void rho(){
    int x, y, m;
    x = 1;
    y = 0;
    for(int t = 0; t < 24; t ++){
        A[x][y] = left_shift(A[x][y], ((t + 1)*(t + 2) / 2) % 64);
        m = x;
        x = y;
        y = (2 * m + 3 * y) % 5;
    }
}

void pi(){
    unsigned long long Ax[5][5];
    memcpy(Ax, A, sizeof(A));

    for(int x = 0;x < 5;x ++){
        for(int y = 0;y < 5;y ++){
            A[x][y] = Ax[(x + 3 * y) % 5][x];
            // printf("%d %d <- %d %d\n", x, y, (x + 3 * y) % 5, x);
        }
    }
}

void chi(){
    unsigned long long Ax[5][5];
    memcpy(Ax, A, sizeof(A));

    for(int x = 0;x < 5;x ++){
        for(int y = 0;y < 5;y ++){
            A[x][y] = Ax[x][y] ^ ((Ax[(x + 1) % 5][y] ^ 0xffffffffffffffffull) & Ax[(x + 2) % 5][y]);
        }
    }
}

int rc(int t){
    if(t % 255 == 0)return 1;
    int R[10];R[0] = 1;
    for(int i = 1;i < 8;i ++)R[i] = 0;
    for(int i = 0;i < t % 255;i ++){
        int Rx[10];
        for(int i = 0;i < 8;i ++)Rx[i] = R[i];
        R[0] = Rx[7];
        for(int i = 1;i < 4;i ++)R[i] = Rx[i - 1];
        R[4] = Rx[3] ^ Rx[7];
        R[5] = Rx[4] ^ Rx[7];
        R[6] = Rx[5] ^ Rx[7];
        R[7] = Rx[6];
    }
    return R[0];
}

void iota(int round){
    unsigned long long RC = 0;
    for(int j = 0;j <= 6;j ++){
        if(rc(j + 7 * round) == 1){
            SET_BIT(RC, (1 << j) - 1);
        }
    }
    // printf("%016llx\n", RC);
    A[0][0] ^= RC;
}

void keccak_p(){
    for(int round = 0;round < 24;round ++){
        // printf("Round %d\n", round);
        theta();
        // printf("After theta:\n");
        // for(int y = 0;y < 5;y ++){
        //     for(int x = 0;x < 5;x ++){
        //         printf("[%d, %d] %016llx\n", x, y, A[x][y]);
        //     }
        // }
        rho();
        // printf("After rho:\n");
        // for(int y = 0;y < 5;y ++){
        //     for(int x = 0;x < 5;x ++){
        //         printf("[%d, %d] %016llx\n", x, y, A[x][y]);
        //     }
        // }
        pi();
        // printf("After pi:\n");
        // for(int y = 0;y < 5;y ++){
        //     for(int x = 0;x < 5;x ++){
        //         printf("[%d, %d] %016llx\n", x, y, A[x][y]);
        //     }
        // }
        chi();
        // printf("After chi:\n");
        // for(int y = 0;y < 5;y ++){
        //     for(int x = 0;x < 5;x ++){
        //         printf("[%d, %d] %016llx\n", x, y, A[x][y]);
        //     }
        // }
        iota(round);
        // printf("After iota:\n");
        // for(int y = 0;y < 5;y ++){
        //     for(int x = 0;x < 5;x ++){
        //         printf("[%d, %d] %016llx\n", x, y, A[x][y]);
        //     }
        // }
    }
}

void sponge(int r, unsigned char* message, int message_len, int d){
    // 1. Let P=N || pad(r, len(N)).
    // 2. Let n=len(P)/r.
    // 3. Let c=br.
    // 4. Let P0, … , Pn-1 be the unique sequence of strings of length r such that P = P0 || … || Pn1.
    // 5. Let S=0 b.
    // 6. For i from 0 to n1, let S=f (S ⊕ (Pi|| 0 c)).
    // 7. Let Z be the empty string.
    // 8. Let Z=Z || Truncr(S).
    // 9. If d≤|Z|, then return Trunc d (Z); else continue.
    // 10. Let S=f(S), and continue with Step 8.
    
    int j = (((-2 - message_len) % r) + r) % r;
    // printf("%d\n", j);
    SET_BIT(message[message_len / 8], message_len % 8);
    SET_BIT(message[(message_len + j + 1)/ 8], (message_len + j + 1) % 8);
    for(int i = 0;i < ((message_len + j + 1) % 8);i ++)UNSET_BIT(message[(message_len + j + 1)/ 8], i);
    for(int i = (message_len / 8) + 1; i < (message_len + j + 1)/ 8;i ++)message[i] = 0;
    for(int i = (message_len % 8) + 1;i < 8;i ++)UNSET_BIT(message[message_len / 8], i);
    message_len += 2 + j;
    // for(int i = 0;i < (message_len / 8 + 1);i ++)printf("%02x ", message[i]);
    // printf("\n%d\n", message_len);

    int n = message_len / r;
    int c = 1600 - r;

    // printf("n %d c %d\n", n, c);

    for(int i = 0; i < n; i ++){
        unsigned long long temp_S[5][5];
        for(int y = 0;y < 5;y ++){
            for(int x = 0;x < 5;x ++){
                temp_S[x][y] = 0;
                for(int j = 0;j < 8;j ++){
                    temp_S[x][y] |= ((40 * y + 8 * x + j) < (r / 8) ? ((unsigned long long)message[(i * r / 8) + 40 * y + 8 * x + j]) : 0) << (j * 8);
                }
            }
        }
        for(int y = 0;y < 5;y ++){
            for(int x = 0;x < 5;x ++){
                A[x][y] ^= temp_S[x][y];
            }
        }
        // for(int y = 0;y < 5;y ++){
        //     for(int x = 0;x < 5;x ++){
        //         printf("[%d, %d] %016llx\n", x, y, A[x][y]);
        //     }
        // }
        keccak_p();
    }
    int x = 0;
    int y = 0;
    int z = 0;
    for(int i = 0;i < d / 8;i ++){
        digest[i] = (A[x][y] & (0xFFllu << z)) >> z;
        z += 8;
        if(z >= 64){
            z = 0;
            x = x + 1;
            if(x >= 5){
                x = 0;
                y = y + 1;
            }
        }
    }
}

void keccak(int c, unsigned char* message, int message_len, int d){
    sponge(1600 - c, message, message_len, d);
}

void sha3_224(unsigned char* message, int message_len){
    UNSET_BIT(message[message_len / 8], message_len % 8);
    SET_BIT(message[(message_len + 1)/ 8], (message_len + 1) % 8);
    message_len += 2;
    keccak(448, message, message_len, 224);
    return ;
}

void sha3_256(unsigned char* message, int message_len){
    UNSET_BIT(message[message_len / 8], message_len % 8);
    SET_BIT(message[(message_len + 1)/ 8], (message_len + 1) % 8);
    message_len += 2;
    keccak(512, message, message_len, 256);
    return ;
}

void sha3_384(unsigned char* message, int message_len){
    UNSET_BIT(message[message_len / 8], message_len % 8);
    SET_BIT(message[(message_len + 1)/ 8], (message_len + 1) % 8);
    message_len += 2;
    keccak(768, message, message_len, 384);
    return ;
}

void sha3_512(unsigned char* message, int message_len){
    UNSET_BIT(message[message_len / 8], message_len % 8);
    SET_BIT(message[(message_len + 1)/ 8], (message_len + 1) % 8);
    message_len += 2;
    keccak(1024, message, message_len, 512);
    return ;
}

void main(int argc, char ** argv){

    unsigned char cooked_message[4*1024*1024+1000];
    int message_len = 0;
    if(argc >= 5 && argv[3][1] == 'm'){
        message_len = strlen(argv[4]);
        for(int i = 0;i < message_len;i ++){
            if(argv[4][i] == '1'){
                SET_BIT(cooked_message[i / 8], i % 8);
            }
            else{
                UNSET_BIT(cooked_message[i / 8], i % 8);
            }
        }
    }
    else if(argc >= 5 && argv[3][1] == 'f'){
        FILE *f = fopen(argv[4], "r");
        fgets(cooked_message, 2000000000,f);
        message_len = strlen(cooked_message) * 8;
    }

    clock_t start;
    clock_t end;
    switch (argv[2][0]){
        case '0':
            start = clock();
            sha3_224(cooked_message, message_len);
            end = clock();
            // printf("digest:\n");
            for(int i = 0;i < 224 / 8;i ++)printf("%02x", digest[i]);
            printf("\nTime Cost(without IO):%f\n", ((double)end - start) / CLOCKS_PER_SEC);
            break;
        case '1':
            start = clock();
            sha3_256(cooked_message, message_len);
            end = clock();
            // printf("digest:\n");
            for(int i = 0;i < 256 / 8;i ++)printf("%02x", digest[i]);
            printf("\nTime Cost(without IO):%f\n", ((double)end - start) / CLOCKS_PER_SEC);
            break;
        case '2':
            start = clock();
            sha3_384(cooked_message, message_len);
            end = clock();
            // printf("digest:\n");
            for(int i = 0;i < 384 / 8;i ++)printf("%02x", digest[i]);
            printf("\nTime Cost(without IO):%f\n", ((double)end - start) / CLOCKS_PER_SEC);
            break;
        case '3':
            start = clock();
            sha3_512(cooked_message, message_len);
            end = clock();
            // printf("digest:\n");
            for(int i = 0;i < 512 / 8;i ++)printf("%02x", digest[i]);
            printf("\nTime Cost(without IO):%f\n", ((double)end - start) / CLOCKS_PER_SEC);
            break;
    }
    return ;
}