#include <math.h>
#include "stm32f4_discovery_audio_codec.h"

/*������ ������*/
#define SIZE 8192
#define FREQ 16000

#define  NUMBER_IS_2_POW_K(x)   ((!((x)&((x)-1)))&&((x)>1))  // x is pow(2, k), k=1,2, ...
#define  FT_DIRECT        -1    // Direct transform.
#define  FT_INVERSE        1    // Inverse transform.
/* ����� ��� ������ � ���������������
 * ������ �������� � 2 ���� ������ ������,
 * ��� ������������ ����������� ����� ������ ���
 * � ������� �������.
 */
uint16_t Audio_buffer[SIZE*2];
//������� ����������� �����
float C_l[SIZE*2];

void  FFT(float *Rdat, float *Idat, int N, int LogN, int Ft_Flag);
void calkulate_MEL_vector();

int main(void)
{
	SystemInit();
	memset(Audio_buffer,0,sizeof(Audio_buffer));
	/*PC2 - ����*/
	/*PA5 - �����*/
	Init_record_and_play(SIZE, //������ ������ ��� ������, ����������� ��������
						FREQ, //������ ������������� �����
						&Audio_buffer[0], //��������� �� ������ ����� ��� ������
						&Audio_buffer[SIZE], //��������� �� ������ ����� ��� ������
						&Audio_buffer[SIZE], //��������� �� ������ ����� ��� ���������������
						&Audio_buffer[0]);	//��������� �� ������ ����� ��� ���������������
	Start_record(); //������ ������
	calkulate_MEL_vector();
	//Start_playing(); //������ ���������������
    while(1)
    {
    }
}
void calkulate_MEL_vector(){
	short i=0;
		short N=SIZE*2;
		float H_k[N];
		float S_m[N];
		float fft_buffer[N];
		//�������� ������
		//������ ����� �������
		for(i;i<N;i++){
			fft_buffer[i]=Audio_buffer[i];
		}
		//���������� ������� �������������� �����
		FFT(fft_buffer, NULL, 8, 3, FT_DIRECT);
		i=0;
		double_t log_expression=0.0;
		//������� ���-�������
		for(i;i<N;i++){
			//���������������
			log_expression=1+((FREQ/2)*i/(N))/700.0;
			//���������� ������� �������, ��� ����������� �������� �� ��������
			fft_buffer[i]=0.54-0.46*cosf(2*3.14*i/(N-1));
			//���������� ������� ���-�������
			H_k[i]=1127*log10(log_expression);
			//���������� ��������
			S_m[i]=log(abs(fft_buffer[i]*fft_buffer[i])*H_k[i]);
			//���������� ��������������
			C_l[i]=S_m[i]*cosf(3.14*i*(i+0.5)/N);
		}
}
//_________________________________________________________________________________________
//_________________________________________________________________________________________
//
// NAME:          FFT.
// PURPOSE:       ������� �������������� �����: ����������� ������ � ����������� ������ � �������.
//                � ������ ��������������� ������� � ������ ����� (Idat) ������������ ����.
//                ���������� �������� - ������ 2**� - �.�. 2, 4, 8, 16, ... (��. ����������� ����).
//
//
// PARAMETERS:
//
//    float *Rdat    [in, out] - Real part of Input and Output Data (Signal or Spectrum)
//    float *Idat    [in, out] - Imaginary part of Input and Output Data (Signal or Spectrum)
//    int    N       [in]      - Input and Output Data length (Number of samples in arrays)
//    int    LogN    [in]      - Logarithm2(N)
//    int    Ft_Flag [in]      - Ft_Flag = FT_ERR_DIRECT  (i.e. -1) - Direct  FFT  (Signal to Spectrum)
//		                 Ft_Flag = FT_ERR_INVERSE (i.e.  1) - Inverse FFT  (Spectrum to Signal)
//
// RETURN VALUE:  false on parameter error, true on success.
//_________________________________________________________________________________________
//
// NOTE: In this algorithm N and LogN can be only:
//       N    = 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384;
//       LogN = 2, 3,  4,  5,  6,   7,   8,   9,   10,   11,   12,   13,    14;
//_________________________________________________________________________________________
//_________________________________________________________________________________________
void  FFT(float *Rdat, float *Idat, int N, int LogN, int Ft_Flag)
{
  // parameters error check:
  //if((Rdat == NULL ) || (Idat == NULL))                  return 0;
  if((N > 16384) || (N < 1))                            return 0;
  if(!NUMBER_IS_2_POW_K(N))                             return 0;
  if((LogN < 2) || (LogN > 14))                         return 0;
  if((Ft_Flag != FT_DIRECT) && (Ft_Flag != FT_INVERSE)) return 0;

  register int  i, j, n, k, io, ie, in, nn;
  float         ru, iu, rtp, itp, rtq, itq, rw, iw, sr;

  static const float Rcoef[14] =
  {  -1.0000000000000000F,  0.0000000000000000F,  0.7071067811865475F,
      0.9238795325112867F,  0.9807852804032304F,  0.9951847266721969F,
      0.9987954562051724F,  0.9996988186962042F,  0.9999247018391445F,
      0.9999811752826011F,  0.9999952938095761F,  0.9999988234517018F,
      0.9999997058628822F,  0.9999999264657178F
  };
  static const float Icoef[14] =
  {   0.0000000000000000F, -1.0000000000000000F, -0.7071067811865474F,
     -0.3826834323650897F, -0.1950903220161282F, -0.0980171403295606F,
     -0.0490676743274180F, -0.0245412285229122F, -0.0122715382857199F,
     -0.0061358846491544F, -0.0030679567629659F, -0.0015339801862847F,
     -0.0007669903187427F, -0.0003834951875714F
  };

  nn = N >> 1;
  ie = N;
  for(n=1; n<=LogN; n++)
  {
    rw = Rcoef[LogN - n];
    iw = Icoef[LogN - n];
    if(Ft_Flag == FT_INVERSE) iw = -iw;
    in = ie >> 1;
    ru = 1.0F;
    iu = 0.0F;
    for(j=0; j<in; j++)
    {
      for(i=j; i<N; i+=ie)
      {
        io       = i + in;
        rtp      = Rdat[i]  + Rdat[io];
        itp      = Idat[i]  + Idat[io];
        rtq      = Rdat[i]  - Rdat[io];
        itq      = Idat[i]  - Idat[io];
        Rdat[io] = rtq * ru - itq * iu;
        Idat[io] = itq * ru + rtq * iu;
        Rdat[i]  = rtp;
        Idat[i]  = itp;
      }

      sr = ru;
      ru = ru * rw - iu * iw;
      iu = iu * rw + sr * iw;
    }

    ie >>= 1;
  }

  for(j=i=1; i<N; i++)
  {
    if(i < j)
    {
      io       = i - 1;
      in       = j - 1;
      rtp      = Rdat[in];
      itp      = Idat[in];
      Rdat[in] = Rdat[io];
      Idat[in] = Idat[io];
      Rdat[io] = rtp;
      Idat[io] = itp;
    }

    k = nn;

    while(k < j)
    {
      j   = j - k;
      k >>= 1;
    }

    j = j + k;
  }

  if(Ft_Flag == FT_DIRECT) return 1;

  rw = 1.0F / N;

  for(i=0; i<N; i++)
  {
    Rdat[i] *= rw;
    Idat[i] *= rw;
  }

  return 1;
}
