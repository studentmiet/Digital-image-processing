// Simplest & slowest version of arithmetic codec (almost demo version)
#include <stdio.h>
#include <process.h>
#include <time.h>
//��� ��������� ������������:	MAX_FREQUENCY * (TOP_VALUE+1) < ULONG_MAX 
//����� MAX_FREQUENCY ������ ���� �� �����, ��� � 4 ���� ������ TOP_VALUE 
//����� �������� NO_OF_CHARS ������ ���� ����� ������ MAX_FREQUENCY 
#define BITS_IN_REGISTER			17	
#define TOP_VALUE					(((unsigned long)1<<BITS_IN_REGISTER)-1)	// 1111...1
#define FIRST_QTR					((TOP_VALUE>>2) +1)							// 0100...0
#define HALF						(2*FIRST_QTR)								// 1000...0
#define THIRD_QTR					(3*FIRST_QTR)								// 1100...0
#define MAX_FREQUENCY				((unsigned)1<<15)
#define NO_OF_CHARS					256
#define EOF_SYMBOL					NO_OF_CHARS			// char-����: 0..NO_OF_CHARS-1 
#define NO_OF_SYMBOLS				(NO_OF_CHARS+1)		// + EOF_SYMBOL

unsigned long						low, high, value;
int									buffer, bits_to_go, garbage_bits, bits_to_follow;
unsigned int						cum_freq[NO_OF_SYMBOLS+1];	//��������� ������ ��������
// ������������� ������� ��������� ������� s (������ ����������� ��� ���������)
// ������������ ��� p(s)=(cum_freq[s+1]-cum_freq[s])/cum_freq[NO_OF_SYMBOLS]

FILE *in, *out; 

void start_model(void)	
{ // ������� ��� ������� � ��������� ������� ��������������� 
	for (int i=0; i<=NO_OF_SYMBOLS; i++)	cum_freq[i]=i;
}
inline void update_model(int symbol)
{
        if (cum_freq[NO_OF_SYMBOLS]==MAX_FREQUENCY) 
        {	//	������������ ��������� ���������, �������� �� � 2 ����
            int cum=0; 
            for (int i=0; i<NO_OF_SYMBOLS; i++)
                {
					int fr=(cum_freq[i+1]-cum_freq[i]+1)>>1;
					cum_freq[i]=cum;
					cum+=fr;
                }
			cum_freq[NO_OF_SYMBOLS]=cum;
        }
		// ���������� ���������� ������
        for (int i=symbol+1; i<=NO_OF_SYMBOLS; i++) cum_freq[i]++;
}

inline int input_bit(void) // ���� 1 ���� �� ������� �����
{
        if (bits_to_go==0)
        {
                buffer=getc(in);	// ��������� ����� �������� �����
                if (buffer==EOF)	// ������� ����� ������ ������ �������� !!!
                {
									// ������� ������� ����������� ������: ��������� 
									// ������������ �������� ������ ���� EOF_SYMBOL,
									// �� ������� �� ���� ���� �� ����� � ����� ���������� 
									// � ����������� �������������, �������� ����� ���� 
									// (��. ���� for(;;) � ��������� decode_symbol). ��� 
									// ���� - "�����", ������� �� ����� ������� 
									// ���������� � �� ����� ������ ������
                        garbage_bits++; 
                        if (garbage_bits>BITS_IN_REGISTER-2)
                        {	// ������ ����������� ���������� ����� �������� �����
                                printf("ERROR IN COMPRESSED FILE !!! \n");
                                exit(-1);
                        }
						bits_to_go=1;
                }
                else bits_to_go=8;
        }
        int t=buffer&1;
        buffer>>=1;
        bits_to_go--;
        return t;
}

inline void output_bit(int bit)			// ����� ������ ���� � ������ ����
{
        buffer=(buffer>>1)+(bit<<7);	// � ������� ����� (���� ����)
        bits_to_go--;
        if (bits_to_go==0)				// ������� ����� ��������, ����� ������
        {
                putc(buffer,out);
                bits_to_go=8;
        }
}
inline void output_bit_plus_follow(int bit)
{		// ����� ������ ���������� ���� � ���, ������� ���� ��������
        output_bit(bit);
        while (bits_to_follow>0)
        {
                output_bit(!bit);
                bits_to_follow--;
        }
}

void start_encoding(void)
{
        bits_to_go		=8;				// �������� ��� � ������� ������ ������
        bits_to_follow	=0;				// ����� ���, ����� ������� �������
        low				=0;				// ������ ������� ���������
        high			=TOP_VALUE;		// ������� ������� ���������
}
void done_encoding(void)
{
        bits_to_follow++;
        if ( low < FIRST_QTR ) output_bit_plus_follow(0);
        else output_bit_plus_follow(1);
        putc(buffer>>bits_to_go,out);				// �������� ������������� �����
}

void start_decoding(void)
{
        bits_to_go		=0;				// �������� ��� � ������� ������ �����
        garbage_bits	=0;				// �������� ����� "��������" ��� � ����� ������� �����
        low				=0;				// ������ ������� ���������
        high			=TOP_VALUE;		// ������� ������� ���������
        value			=0;				// "�����"
        for (int i=0; i<BITS_IN_REGISTER; i++) value=(value<<1)+input_bit();
}

void encode_symbol(int symbol)
{
		// �������� ������ ���������
        unsigned long range;
        range=high-low+1;
        high=low	+range*cum_freq[symbol+1]/cum_freq[NO_OF_SYMBOLS] -1;
        low	=low	+range*cum_freq[symbol]/cum_freq[NO_OF_SYMBOLS];
		// ����� ��� ������������� - ����� ���� ��� ���� �� ������������
        for (;;)
        {			// ���������: ������ low<high
                if (high<HALF) // ������� ���� low � high - ������� (���)
									output_bit_plus_follow(0); //����� ������������ �������� ����
                else if (low>=HALF) // ������� ���� low � high - ���������
                {	 
                        output_bit_plus_follow(1);	//����� �������� ����
                        low-=HALF;					//����� �������� ���� � 0
                        high-=HALF;					//����� �������� ���� � 0
                }
                else if (low>=FIRST_QTR && high < THIRD_QTR)
                {		/* �������� ������������, �.�. 
								 HALF<=high<THIRD_QTR,	i.e. high=10...
								 FIRST_QTR<=low<HALF,	i.e. low =01...
							����������� ������ �� ����������� ���	*/
                        high-=FIRST_QTR;		// high	=01...
                        low-=FIRST_QTR;			// low	=00...
                        bits_to_follow++;		//����������� ����� (���) ������ ����
						// ������� ��� ����� ������ �����
                }
                else break;		// ��������� ����� ��� ���� 
				//	������� ��� � low � high �������, ��������� ����� ��� � ������� ������ 
                low<<=1;			// ��������� 0
                (high<<=1)++;		// ��������� 1
        }
}

int decode_symbol(void)
{
        unsigned long range, cum;
        int symbol;
        range=high-low+1;
					// ����� cum - ��� ����� value, ������������� �� ���������
					// low..high � �������� 0..CUM_FREQUENCY[NO_OF_SYMBOLS]
		cum=( (value-low+1)*cum_freq[NO_OF_SYMBOLS] -1 )/range;
					// ����� ���������, ���������������� ����� cum
		for (symbol=0; cum_freq[symbol+1] <= cum; symbol++);
					// �������� ������
        high=low +range*cum_freq[symbol+1]/cum_freq[NO_OF_SYMBOLS] -1;
        low =low +range*cum_freq[symbol]/cum_freq[NO_OF_SYMBOLS];
        for (;;)
        {		// ���������� � ������������� ��������� ��������
                if (high<HALF) {/* ������� ���� low � high - ������� */}
                else if (low>=HALF) 
                {		// ������� ���� low � high - ���������, ����������
                        value-=HALF;
                        low-=HALF;
                        high-=HALF;
                }
                else if (low>=FIRST_QTR && high<THIRD_QTR)
                {		// ��������� ��� ��, ��� ��� �����������
                        value-=FIRST_QTR;
                        low-=FIRST_QTR;
                        high-=FIRST_QTR;
                }
                else break;	// ��������� ����� ��� ���� 
				low<<=1;							// ��������� ����� ��� 0
                (high<<=1)++;						// ��������� ����� ��� 1
                value=(value<<1)+input_bit();		// ��������� ����� ��� ����������
        }
        return symbol;
}

void encode(void)
{
        int symbol;
        start_model();
        start_encoding();
        while ( (symbol=getc(in))!=EOF )
        {
                encode_symbol(symbol);
                update_model(symbol);
        }
        encode_symbol(EOF_SYMBOL);
        done_encoding();
}

void decode(void)
{
        int symbol;
        start_model();
        start_decoding();
        while ((symbol=decode_symbol())!=EOF_SYMBOL)
        {
                update_model(symbol);
                putc(symbol,out);
        }
}

void _cdecl main(int argc, char **argv)
{
				printf("\nAlpha version of arithmetic Codec\n");
				if ( argc!=4 ||  argv[1][0]!='e' && argv[1][0]!='d' )
						printf("\nUsage: arcode e|d infile outfile \n");
				else if ( (in=fopen(argv[2],"r+b"))==NULL )
						printf("\nIncorrect input file\n");
				else if ( (out=fopen(argv[3],"w+b"))==NULL )
						printf("\nIncorrect output file\n");
				else 
				{
                    if (argv[1][0] == 'e')
                    {
                        clock_t start = clock();
                        encode();
                        double seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
                        printf("The time: %f seconds\n", seconds);
                    }
                    else
                    {
                        clock_t start = clock();
                        decode();
                        double seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
                        printf("The time: %f seconds\n", seconds);
                    }
					fclose(in);
					fclose(out);
				}
}