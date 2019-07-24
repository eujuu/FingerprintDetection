void CSub_ProjectDoc::OnFingerprint()
{
	int i, j, num_C, value;

	unsigned char LOW, HIGH, Temp;
	double m_HIST[256];
	double m_Sum_Of_HIST[256];
	unsigned char m_Scale_HIST[256];
	unsigned char* dil_Image, *ero_Image, *con_Image;
	double SUM = 0.0;


	m_height = dibHi.biHeight;
	m_width = dibHi.biWidth;
	m_size = m_height * m_width;

	if (dibHi.biBitCount == 24)
		num_C = 3;
	else if (dibHi.biBitCount == 8)
		num_C = 1;

	if (dibHi.biBitCount == 8) {
		m_Re_height = m_height;
		m_Re_width = m_width;
		m_Re_size = m_Re_height * m_Re_width;

		m_OutputImage = new BYTE[m_Re_size * 3];
		mid_Image = new unsigned char[m_size];
		dil_Image = new unsigned char[m_size];
		ero_Image = new unsigned char[m_size];
		con_Image = new unsigned char[m_size];
		
		//Equalization
		LOW = 0;
		HIGH = 255;
	
		for (i = 0; i < 256; i++) {
			m_HIST[i] = LOW; //초기화
		}

		for (i = 0; i < m_size; i++) {
			value = (int)m_InImg[i];
			m_HIST[value]++; // 빈도수 조사
		}

		for (i = 0; i < 256; i++) {
			SUM += m_HIST[i]; //합을 조사
			m_Sum_Of_HIST[i] = SUM; //쓰레기 값이 저장될 수도 있으니 초기화를 해준다.
									//존재하는 밝기 값을 최대한 고르게 분포되게 한다.
		}
		for (i = 0; i < m_size; i++) {
			Temp = m_InImg[i];
			mid_Image[i] = (unsigned char)(m_Sum_Of_HIST[Temp] * HIGH / m_size);
		}
		

		mid_Image = OnMedianSub_bw(mid_Image);


		m_tempImage = Image2DMem(m_height, m_width);
		for (i = 0; i < m_height; i++) {
			for (j = 0; j < m_width; j++) {
				m_tempImage[i][j] = mid_Image[i*m_width + j];
			}
		}

		//block binarization
		double threshold = 0.0;
		for (i = 0; i < m_height; i = i + 5) {
			for (j = 0; j < m_width; j = j + 5)
			{
				SUM = 0.0;
				for (int r = 0; r < 5; r++) {
					for (int s = 0; s < 5; s++) {
						SUM += m_tempImage[i + r][j + s];
					}
				}
				threshold = SUM / 25;
				for (int r = 0; r < 5; r++) {
					for (int s = 0; s < 5; s++) {
						if (m_tempImage[i + r][j + s] > threshold)
							m_tempImage[i + r][j + s] = 255;
						else
							m_tempImage[i + r][j + s] = 0;
					}
				}

			}
		}



		for (i = 0; i < m_height; i++) {
			for (j = 0; j < m_width; j++) {
				mid_Image[i*m_width + j] = m_tempImage[i][j];
			}
		}
		//Filtering
		mid_Image = OnMedianSub_bw(mid_Image);
		con_Image = mid_Image;
		dil_Image = OnDilation_bw(mid_Image);
		mid_Image = OnErosion_bw(mid_Image); 
		mid_Image = OnDilation_bw(mid_Image);

		for (i = 0; i < m_height; i++) {
			for (j = 0; j < m_width; j++) {
				m_tempImage[i][j] = mid_Image[i*m_width + j];
			}
		}
		//Thinning
		m_tempImage = Thin(mid_Image);
		
		//Clamping
		for (i = 0; i < m_height; i++) {
			for (j = 0; j < m_width; j++) {
				if (m_tempImage[i][j] > 255)
					m_tempImage[i][j] = 255.;
				if (m_tempImage[i][j] < 0.)
					m_tempImage[i][j] = 0.;
			}
		}
		
		for (i = 0; i < m_size; i++) {
			m_OutputImage[i * 3] = m_InImg[i];
			m_OutputImage[i * 3 + 1] = m_InImg[i];
			m_OutputImage[i * 3 + 2] = m_InImg[i];
		}
		

		for (i = 0; i < m_height; i++) {
			for (j = 0; j < m_width; j++) {
				mid_Image[i*m_width + j] = m_tempImage[i][j];
			}
		}
		//algorithm for searching features in finger print.
		int search;
		for (i = 50; i < m_height - 50; i++) {
			for (j = 50; j< m_width - 50; j++) {
				search = Searching(mid_Image, i, j);
				if(search){
					for (int k = 0; k < 4; k++) {
						for (int l = 0; l < 4; l++) {
						//color setting
							m_OutputImage[((i + k) * m_Re_width + j + l) * 3 + 0] = 221;
							m_OutputImage[((i + k) * m_Re_width + j + l) * 3 + 1] = 0;
							m_OutputImage[((i + k) * m_Re_width + j + l) * 3 + 2] = 255;
						}
					}
					j = j + 4;
				}
			}
		}
	}
}

int CSub_ProjectDoc::Searching(unsigned char* m_Img, int i, int j)
{
	double Searching_M[3][3] = { {0.,255.,0.,},{255.,255.,255.,},{0.,255.,0.} };
	int color = 220, count = 0, max = 0, max_i, max_j, sear = 0;
	int r, s;
	double img;
	double *Mask = new double[16]; //3*3mask
	int M = 4, n, m;
	for (r = -1; r < 2; r++) {
		for (s = -1; s < 2; s++) {
			if (Searching_M[r + 1][s + 1] - m_Img[(i + r) * m_width + j + s] == 0)
				sear++;
		}
	}
	if (sear >=8)
		return 1;
	else
		return 0;
}
