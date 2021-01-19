
#include "palisade.h"
#include <iostream>
#include <fstream>
#include <map>
#include <time.h>
#include <stdlib.h>
#include "csvstream.h"
#include "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/cnpy.h"
#include <sstream>
#include <chrono>
#include "math/matrix.h"
#include "math/matrix.cpp"

using namespace std;
using namespace lbcrypto;
using namespace std::chrono;


int main() {

  cout << "(1) instantiating crypto-context" << endl;

  auto start = high_resolution_clock::now();

  usint init_size = 4;
  usint dcrtBits = 40;
  usint batchSize = 16;

  CryptoContext<DCRTPoly> cc =
      CryptoContextFactory<DCRTPoly>::genCryptoContextCKKS(
          init_size - 1, dcrtBits, batchSize, HEStd_128_classic,
          0,                    /*ringDimension*/
          APPROXRESCALE, BV, 2, /*numLargeDigits*/
          2,                    /*maxDepth*/
          60,                   /*firstMod*/
          5, OPTIMIZED);

  // enable features that you wish to use
  cc->Enable(ENCRYPTION);
  cc->Enable(SHE);
  cc->Enable(LEVELEDSHE);
  cc->Enable(MULTIPARTY);

  // Initialize Public Key Containers
  LPKeyPair<DCRTPoly> kp1;

  LPKeyPair<DCRTPoly> kpMultiparty;

  kp1 = cc->KeyGen();
  cc->EvalMultKeyGen(kp1.secretKey);

  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " milliseconds" << endl;

  cout << "(2) reading in .npz files & converting to plaintexts!" << endl;

  start = high_resolution_clock::now();



  cnpy::npz_t my_npz = cnpy::npz_load("/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/test_data/learners_flattened/learner1_0.npz");


  float training_samples = 1000;

  vector<string> arrays = {"arr_0", "arr_1", "arr_2", "arr_3", "arr_4", "arr_5", "arr_6", "arr_7", "arr_8", "arr_9", "arr_10","arr_11","arr_12", "arr_13"};

  map<string, Plaintext> mappings;

  for (string idx: arrays) {

      cnpy::NpyArray arr = my_npz[idx];

      cout << idx << ", " << arr.shape << endl;

      float* loaded_data = arr.data<float>();

      vector<float> curr;

      for (int i = 0; i < arr.shape[0]; i++) {
        curr.push_back(loaded_data[i]);
      }

      vector<complex<double>> curr2(curr.begin(), curr.end());

      Plaintext p = cc->MakeCKKSPackedPlaintext(curr2);

      mappings[idx] = p;
  }


  cout << mappings.size() << endl;

  stop = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " milliseconds" << endl;

  cout << "(3) encrypting data!" << endl;

  start = high_resolution_clock::now();

  vector<Ciphertext<DCRTPoly>> ciphertexts;


  map<string, Plaintext>::iterator it;
  for (it = mappings.begin(); it != mappings.end(); it++) {
    Ciphertext<DCRTPoly> ciphertext =
        cc->Encrypt(kp1.publicKey, it->second);
    ciphertexts.push_back(ciphertext);
  }

  stop = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " milliseconds" << endl;

  cout << "(4) running PWA algorithm!" << endl;

  start = high_resolution_clock::now();

  float weight = training_samples / (mappings.size() * training_samples);
  cout << weight << endl;
  auto pwa = cc->EvalMult(ciphertexts[0], weight);

  for (int i = 1; i < 10; i++) {
    pwa = cc->EvalMult(ciphertexts[0], weight);
  }

  for (int i = 1; i < mappings.size(); i++) {
    for (int j = 0; j < 10; j++) {
      pwa += cc->EvalMult(ciphertexts[i], weight);
    }
  }

  stop = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " milliseconds" << endl;

  cout << "(5) decrypting and determining pwa!" << endl;

  start = high_resolution_clock::now();

  Plaintext decryptResult;
  cc->Decrypt(kp1.secretKey, pwa, &decryptResult);
  auto dr = decryptResult->GetCKKSPackedValue();
  //cout << dr << endl;

  duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " milliseconds" << endl;



}





