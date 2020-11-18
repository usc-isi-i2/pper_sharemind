
#include "palisade.h"
#include <iostream>
#include <fstream>
#include <map>
#include <time.h>
#include <stdlib.h>
#include "csvstream.h"
#include <sstream>
#include <chrono>

using namespace std;
using namespace lbcrypto;
using namespace std::chrono;

int main() {
  // Instantiate the crypto context

  cout << "(1) gen crypto-context!" << endl;

  auto start = high_resolution_clock::now();

  SecurityLevel securityLevel = HEStd_128_classic;
  usint init_size = 4;
  usint dcrtBits = 40;
  usint batchSize = 16;

  CryptoContext<DCRTPoly> cc =
      CryptoContextFactory<DCRTPoly>::genCryptoContextCKKS(
          init_size - 1, dcrtBits, batchSize, securityLevel,
          0,                    /*ringDimension*/
          APPROXRESCALE, BV, 2, /*numLargeDigits*/
          2,                    /*maxDepth*/
          60,                   /*firstMod*/
          5, OPTIMIZED);

  LPKeyPair<DCRTPoly> keyPair;

  cc->Enable(ENCRYPTION);
  cc->Enable(SHE);
  cc->Enable(LEVELEDSHE);

  keyPair = cc->KeyGen();
  cc->EvalMultKeyGen(keyPair.secretKey);

  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " ms" << endl;

  cout << "(2) converting to plaintexts!" << endl;

  start = high_resolution_clock::now();


  // weights
  vector<complex<double>> weights = {0.3, 0.2, 0.1, 0.4};

  // user input
  vector<complex<double>> input = {1, 2, 3, 5};

  Plaintext p_weights;
  Plaintext p_input;

  p_weights = (cc->MakeCKKSPackedPlaintext(weights));

  p_input = (cc->MakeCKKSPackedPlaintext(input));

  stop = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " ms" << endl;

  cout << "(2) encrypting weights and input data!" << endl;

  start = high_resolution_clock::now();

  Ciphertext<DCRTPoly> c_weights;
  c_weights = (cc->Encrypt(keyPair.publicKey, p_weights));

  Ciphertext<DCRTPoly> c_input;
  c_input = (cc->Encrypt(keyPair.publicKey, p_input));

  stop = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " ms" << endl;

  cout << "(3) calculating homomorphic weighted average!" << endl;

  start = high_resolution_clock::now();

  auto sum = cc->EvalMult(c_weights, c_input);

  stop = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " ms" << endl;

  cout << "(4) decrypting result!" << endl;

  Plaintext decryptResult;
  cc->Decrypt(keyPair.secretKey, sum, &decryptResult);
  decryptResult->SetLength(p_weights->GetLength());

  double weighted_average = 0;
  auto dr = decryptResult->GetCKKSPackedValue();
  for (int i = 0; i < dr.size(); i++) {
    weighted_average += dr[i].real();
  }
  cout << weighted_average << endl;



}
