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

int x_1[5] = {1, 2, 3, 4, 0};
int x_2[5] = {2, 3, 4, 0, 1};
int x__1[5] = {4, 0, 1, 2, 3};

void theta(){
    unsigned long long C[5], D[5];

    memset(C, 0, sizeof(C));
    memset(D, 0, sizeof(D));

    for(int x = 0;x < 5;x ++){
        C[x] = A[x][0] ^ A[x][1] ^ A[x][2] ^ A[x][3] ^ A[x][4];
    }

    for(int x = 0;x < 5;x ++){
        D[x] = C[x__1[x]] ^ left_shift(C[x_1[x]], 1);
    }

    for(int x = 0;x < 5;x ++){
        for(int y = 0;y < 5;y ++){
            A[x][y] = A[x][y] ^ D[x];
        }
    }
}

int rho_table[24] = {1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14, 27, 41, 56, 8, 25, 43, 62, 18, 39, 61, 20, 44};
int rho_x[24] = {1, 0, 2, 1, 2, 3, 3, 0, 1, 3, 1, 4, 4, 0, 3, 4, 3, 2, 2, 0, 4, 2, 4, 1};
int rho_y[24] = {0, 2, 1, 2, 3, 3, 0, 1, 3, 1, 4, 4, 0, 3, 4, 3, 2, 2, 0, 4, 2, 4, 1, 1};

void rho(){
    for(int t = 0; t < 24; t ++){
        A[rho_x[t]][rho_y[t]] = left_shift(A[rho_x[t]][rho_y[t]], rho_table[t]);
    }
}

int pi_x[5][5] = {0, 3, 1, 4, 2, 1, 4, 2, 0, 3, 2, 0, 3, 1, 4, 3, 1, 4, 2, 0, 4, 2, 0, 3, 1};

void pi(){
    unsigned long long Ax[5][5];
    memcpy(Ax, A, sizeof(A));

    for(int x = 0;x < 5;x ++){
        for(int y = 0;y < 5;y ++){
            A[x][y] = Ax[pi_x[x][y]][x];
        }
    }
}

void chi(){
    unsigned long long Ax[5][5];
    memcpy(Ax, A, sizeof(A));

    for(int x = 0;x < 5;x ++){
        for(int y = 0;y < 5;y ++){
            A[x][y] = Ax[x][y] ^ ((Ax[x_1[x]][y] ^ 0xffffffffffffffffull) & Ax[x_2[x]][y]);
        }
    }
}

unsigned long long RC[24] = {
    0x0000000000000001,
    0x0000000000008082,
    0x800000000000808a,
    0x8000000080008000,
    0x000000000000808b,
    0x0000000080000001,
    0x8000000080008081,
    0x8000000000008009,
    0x000000000000008a,
    0x0000000000000088,
    0x0000000080008009,
    0x000000008000000a,
    0x000000008000808b,
    0x800000000000008b,
    0x8000000000008089,
    0x8000000000008003,
    0x8000000000008002,
    0x8000000000000080,
    0x000000000000800a,
    0x800000008000000a,
    0x8000000080008081,
    0x8000000000008080,
    0x0000000080000001,
    0x8000000080008008
};

void iota(int round){
    A[0][0] ^= RC[round];
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