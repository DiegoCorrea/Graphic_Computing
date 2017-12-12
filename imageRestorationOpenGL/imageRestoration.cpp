# include <GL/glut.h>
# include <iostream>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <math.h>
# define PI 3.14
using namespace std;

const bool LOGGER = true;
FILE *fl_LOGGER;



// BMP
int channels = 3;           // RGB channels
unsigned char header[54];   // Each BMP file begins by a 54-bytes header
unsigned int dataPos;       // Position in the file where the actual data begins
unsigned int width, height; // Image width and height
unsigned int imageSize;     // width*height*channels
unsigned char* data;        // Actual RGB data
unsigned char* newImageData; // Actual RGB data
int grayLevel[256] = {0};

const int phiS = 3;
const int phiR = 8;
const int pixelWindowRange = 9*9;

float gaussianFunction(int distance, int factor) {
	float eulerNumber = exp(-(pow(distance/(2*factor), 2.0)));
	//float eulerNumber = exp(-(pow(distance, 2.0)/(2*pow(factor, 2.0))));
	float firstPart = 1/(factor*sqrt(2*PI));
	//float firstPart = 1/(2*(3.14*(factor^2)));
	float result = firstPart*eulerNumber;

	// <LOGGER>
	if (LOGGER){
		fprintf(fl_LOGGER, "[gaussianFunction] - Iniciando\n");
		fprintf(fl_LOGGER, "\t[gaussianFunction] - O Valor do numero de Euler: %f\n", eulerNumber);
		fprintf(fl_LOGGER, "\t[gaussianFunction] - Valor da primeira parte: %f\n", firstPart);
		fprintf(fl_LOGGER, "\t[gaussianFunction] - Resultado da função Gaugasiana: %f\n", result);
		fprintf(fl_LOGGER, "[gaussianFunction] - Finalizando\n");
	} // </LOGGER>
	return result;
}

float normalizationFactor(int processPixelPosition) {
	float result = 0;
	int linePosition = 0, columnPosition = 0;
	// <LOGGER>
	if (LOGGER){
		fprintf(fl_LOGGER, "\n[normalizationFactor] - Iniciando\n");
	}// </LOGGER>
	for(int line = -4; line <= 4; line++){
		linePosition = line*3;
		for(int column = -4; column <= 4; column++){
			columnPosition = column*3;
			// <LOGGER>
			if (LOGGER){
				fprintf(fl_LOGGER, "\t[normalizationFactor] - Linha: %d || Coluna: %d\n", line, column);
				fprintf(fl_LOGGER, "\t[normalizationFactor] - Posição da Linha no data: %d || Posição da Coluna no data: %d\n", linePosition, columnPosition);
			}// </LOGGER>
			int processWindowPosition = (linePosition*width) + columnPosition; // Calculando a distância 9 x 9
			processWindowPosition = processPixelPosition + processWindowPosition;

			if (processWindowPosition < 0 || processWindowPosition > imageSize){
				// <LOGGER>
				if (LOGGER){
					fprintf(fl_LOGGER, "\t[normalizationFactor] - Valor fora da matrix\n");
				}// </LOGGER>
				result += gaussianFunction(abs(processPixelPosition - processPixelPosition), phiS) * gaussianFunction((data[processPixelPosition] - data[processPixelPosition]), phiR);
			} else {
				result += gaussianFunction(abs(processPixelPosition - processWindowPosition), phiS) * gaussianFunction((data[processPixelPosition] - data[processWindowPosition]), phiR);
			}
		}
	}
	// <LOGGER>
	if (LOGGER){
		fprintf(fl_LOGGER, "[normalizationFactor] - Finalizando\n");
	}// </LOGGER>
	return result;
}

float bfNormalizationFactorWithExtra(int processPixelPosition) {
	float result = 0;
	int linePosition = 0, columnPosition = 0;
	// <LOGGER>
	if (LOGGER){
		fprintf(fl_LOGGER, "\n[bfNormalizationFactorWithExtra] - Iniciando\n");
	}// </LOGGER>
	for(int line = -4; line <= 4; line++){
		linePosition = line*3;
		for(int column = -4; column <= 4; column++){
			columnPosition = column*3;
			// <LOGGER>
			if (LOGGER){
				fprintf(fl_LOGGER, "\t[bfNormalizationFactorWithExtra] - Linha: %d || Coluna: %d\n", line, column);
				fprintf(fl_LOGGER, "\t[bfNormalizationFactorWithExtra] - Posição da Linha no data: %d || Posição da Coluna no data: %d\n", linePosition, columnPosition);
			}// </LOGGER>
			int processWindowPosition = (linePosition*width) + columnPosition; // Calculando a distância 9 x 9
			processWindowPosition = processPixelPosition + processWindowPosition;

			if (processWindowPosition < 0 || processWindowPosition > imageSize){
				// <LOGGER>
				if (LOGGER){
					fprintf(fl_LOGGER, "[bfNormalizationFactorWithExtra] - Valor fora da matrix\n");
				}// </LOGGER>
				result += gaussianFunction(abs(processPixelPosition - processPixelPosition), phiS) * gaussianFunction((data[processPixelPosition] - data[processPixelPosition]), phiR);
			} else {
				result += gaussianFunction(abs(processPixelPosition - processWindowPosition), phiS) * gaussianFunction((data[processPixelPosition] - data[processWindowPosition]), phiR) * data[processWindowPosition];
			}
		}
	}
	// <LOGGER>
	if (LOGGER){
		fprintf(fl_LOGGER, "[bfNormalizationFactorWithExtra] - Finalizando\n");
	}// </LOGGER>
	return result;
}

void bilateralFilter(){
	newImageData = (unsigned char *)malloc(imageSize);

	// <LOGGER>
	if (LOGGER){
		fprintf(fl_LOGGER, "\n[bilateralFilter] - Iniciando\n");
	}// </LOGGER>

	for(int p = 0; p < width * height ; p++){
		int index = p*3;
		// <LOGGER>
		if (LOGGER){
			fprintf(fl_LOGGER, "\t[bilateralFilter] - Pixel é: %d de %d, Posição Avaliada %d\n", p, (width * height), index);
		}// </LOGGER>
		float Wp = 1/normalizationFactor(index);
		unsigned char BF = (unsigned char)(bfNormalizationFactorWithExtra(index)/Wp);
		newImageData[index] = BF;
		newImageData[index+1] = BF;
		newImageData[index+2] = BF;

	}

	// <LOGGER>
	if (LOGGER){
		fprintf(fl_LOGGER, "[bilateralFilter] - Finalizando\n");
	}// </LOGGER>
}

int loadImageBMP(const char *imagepath) {
	// Open the file
	FILE * file = fopen(imagepath,"rb");
	if (!file){
		printf("O arquivo não pode ser aberto\n"); 
		return 0;
	}

	if ( fread(header, 1, 54, file)!=54 ) { // If not 54 bytes read : problem
		printf("Not a correct BMP file\n");
		return 0;
	}

	if ( header[0]!='B' || header[1]!='M' ) {
		printf("Not a correct BMP file\n");
		return 0;
	}

	// Read ints from the byte array
	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);
	// <LOGGER>
	if (LOGGER){
		fprintf(fl_LOGGER, "[loadImageBMP] - Iniciando\n");
		fprintf(fl_LOGGER, "\t[loadImageBMP] - Posição inicial do arquivo de imagem: %d\n", dataPos);
		fprintf(fl_LOGGER, "\t[loadImageBMP] - Tamanho da imagem em KB: %d\n", imageSize);
		fprintf(fl_LOGGER, "\t[loadImageBMP] - Comprimento da imagem: %d\n", width);
		fprintf(fl_LOGGER, "\t[loadImageBMP] - Altura da imagem: %d\n", height);
		fprintf(fl_LOGGER, "[loadImageBMP] - Finalizando\n");
	} // </LOGGER>

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)
		imageSize = width*height*channels; // 3 channels : one byte for each Red, Green and Blue component
	if (dataPos == 0)
		dataPos = 54; // The BMP header is done that way


	// Create a buffer
	data = (unsigned char *)malloc(imageSize);

	// Read the actual data from the file into the buffer
	fread(data,1,imageSize,file);

	int i;
	for(i = 0; i < width * height ; ++i) {
		int index = i*3;
		unsigned char R, G, B;
		B = data[index];
		R = data[index+2];
		data[index] = R;
		data[index+2] = B;
	}

	//Everything is in memory now, the file can be closed
	fclose(file);
}

void init(void) {
  //glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glClearColor (0.0, 0.0, 0.0, 0.0);
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT);
  //glColor3f (1.0, 1.0, 1.0);

  glRasterPos2i (0, 0);
  glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

  //cinzao();

  glRasterPos2i (width, 0);
  glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, newImageData);
  glFlush();
}

void reshape(int w, int h) {
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho (0, w, 0, h, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
}
void keyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 27:
      exit(0);
  }
}
char *getImageName(char **argv) {
	// <LOGGER>
	if (LOGGER){
		fprintf(fl_LOGGER, "[getImageName] - Iniciando\n");
	}// </LOGGER>

	int len = strlen(argv[1]);
    char *fileName = new char[len];
	strncpy(fileName, argv[1], len-4);	
	// <LOGGER>
	if (LOGGER){
		fprintf(fl_LOGGER, "\t[getImageName] - Nome da Imagem é: %s\n", fileName);
		fprintf(fl_LOGGER, "\t[getImageName] - Finalizando\n");
	}// </LOGGER>
	return fileName;
}

int main(int argc, char **argv) {
	if(argc < 2){
		printf("Uma imagem é necessaria para aplicar o Filtro e a Máscara\n");
		return 1;
	}
	// <LOGGER>
	if (LOGGER){
		fl_LOGGER = fopen("imageRestoration.log", "w+" );
		fprintf(fl_LOGGER, "[main] - Iniciando Restauração de Imagem\n");
	} // </LOGGER>
	char *fileName = getImageName(argv);

	loadImageBMP(argv[1]);
	bilateralFilter();
	//for(int i = 0; i < 10000;i+=1000)
		//normalizationFactor(0);

	// <LOGGER>
	if (LOGGER){
		fprintf(fl_LOGGER, "[main] - Finalizando processamento e exibindo tela\n");
		fclose( fl_LOGGER );
	} // </LOGGER>

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(width*2, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	init();
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(display);
	glutMainLoop();

	return 0;
}