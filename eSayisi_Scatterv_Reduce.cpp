#include <mpi.h>
#include <iostream>
#include <malloc.h>
#include <math.h>
#include <iomanip>
using namespace std;

int faktoriyel(int x) {
	if (x == 1 || x == 0) {
		return 1;
	}
	else {
		return x * faktoriyel(x - 1);
	}
}

int main(int argc, char* argv[]) {

	int size = 0,
		myRank = 0,
		boyut = 0,
		bolum = 0,
		kalan = 0,
		chunkSize = 0;

	int* sendBuffer = nullptr,
		* receiveBuffer = nullptr,
		* displs = nullptr,
		* sendCounts = nullptr;

	double	baslangic = 0.0,
		bitis = 0.0,
		lokalToplam = 0.0,
		genelToplam = 0.0;

	MPI_Status status;

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	if (myRank == 0) {
		cout << "Boyutu giriniz : ";
		cin >> boyut;
		cout << endl;

		baslangic = MPI_Wtime();
		boyut++;
		sendBuffer = new int[boyut];

		for (int i = 0; i < boyut; i++) {
			sendBuffer[i] = i;
		}

		bolum = floor(boyut / size);
		kalan = floor(boyut % size);

		sendCounts = new int[size];
		displs = new int[size];

		/*
			Hesaplama algoritmasi olarak Blok Dagitim Teknigi III kullanýlacak. Bu teknikte boyut hesaplama kumesine
			tam bolunmuyor ve fazlalik olan verilerde yuk dengesi gozetiliyor.(Butun fazlalik tek bir islemciye ve-
			rilmiyor!)
		*/

		for (int i = 0; i < size; i++) {
			if (i < kalan) {
				sendCounts[i] = ceil((double)boyut / (double)size);
			}
			else {
				sendCounts[i] = bolum;
			}
		}

		displs[0] = 0;

		for (int i = 1; i < size; i++) {
			if (i <= kalan) {
				displs[i] = (displs[i - 1] + ((int)ceil((double)(boyut / size)))) + 1;
			}
			else if (i > kalan) {
				displs[i] = (displs[i - 1] + bolum);
			}
		}

		chunkSize = sendCounts[0];

		for (int i = 1; i < size; i++) {
			MPI_Send(&sendCounts[i], 1, MPI_INT, i, 25, MPI_COMM_WORLD);
		}

	}
	else {
		MPI_Recv(&chunkSize, 1, MPI_INT, 0, 25, MPI_COMM_WORLD, &status);
	}

	receiveBuffer = new int[chunkSize];

	MPI_Scatterv(sendBuffer, sendCounts, displs, MPI_INT, receiveBuffer, chunkSize, MPI_INT, 0, MPI_COMM_WORLD);

	for (int i = 0; i < chunkSize; i++) {
		lokalToplam = lokalToplam + (1.0 / (double)faktoriyel(receiveBuffer[i]));
	}

	MPI_Reduce(&lokalToplam, &genelToplam, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	if (myRank == 0) {
		delete[] sendBuffer;

		cout << "e sayisi : " << setprecision(10) << genelToplam << endl;
	}

	delete[] receiveBuffer;

	bitis = MPI_Wtime();
	MPI_Finalize();

	if (myRank == 0) {
		cout << "Gecen toplam sure : " << setprecision(5) << bitis - baslangic << " s" << endl;
	}

	return 0;
}
