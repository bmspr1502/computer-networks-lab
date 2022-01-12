#include<stdio.h>
#include<iostream>
#include<math.h>

using namespace std;

string toBin(long long int num){
	string bin = "";
	while (num){
		if (num & 1)
			bin = "1" + bin;
		else
			bin = "0" + bin;
		num = num>>1;
	}
	return bin;
}

long long int toDec(string bin){
	long long int num = 0;
	for (int i=0; i<bin.length(); i++){
		if (bin.at(i)=='1')
			num += 1 << (bin.length() - i - 1);
	}
	return num;
}

void CRC(string dataword, string generator){
	int l_gen = generator.length();
	long long int gen = toDec(generator);

	long long int dword = toDec(dataword);

	long long int dividend = dword << (l_gen-1);	

	int shft = (int) ceill(log2l(dividend+1)) - l_gen;
	long long int rem;

	while ((dividend >= gen) || (shft >= 0)){
		rem = (dividend >> shft) ^ gen;			
		dividend = (dividend & ((1 << shft) - 1)) | (rem << shft);
		shft = (int) ceill(log2l(dividend + 1)) - l_gen;
	}
	long long int codeword = (dword << (l_gen - 1)) | dividend;
	cout << "Remainder: " << toBin(dividend) << endl;
	cout << "Encoded data : " << toBin(codeword) << endl;
}

int main(){
	string dataword, generator;
	cout<<"Enter the dataword: "<<endl;
	cin>>dataword;
	cout<<"Enter the generator: "<<endl;
	cin>>generator;
	CRC(dataword, generator);
	return 0;
}


