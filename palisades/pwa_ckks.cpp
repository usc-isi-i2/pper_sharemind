
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

  cout << "reading in learner 1!" << endl;
  cnpy::npz_t l1 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners/l1.npz");

  cout << "reading in learner 2!" << endl;
  cnpy::npz_t l2 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners/l2.npz");

  cout << "reading in learner 3!" << endl;
  cnpy::npz_t l3 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners/l3.npz");

  cout << "reading in learner 4!" << endl;
  cnpy::npz_t l4 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners/l4.npz");

  cout << "reading in learner 5!" << endl;
  cnpy::npz_t l5 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners/l5.npz");

  cout << "reading in learner 6!" << endl;
  cnpy::npz_t l6 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners/l6.npz");

  cout << "reading in learner 7!" << endl;
  cnpy::npz_t l7 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners/l7.npz");

  cout << "reading in learner 8!" << endl;
  cnpy::npz_t l8 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners/l8.npz");

  float training_samples = 1000;

  vector<cnpy::npz_t> learners = {l1, l2, l3, l4, l5, l6, l7, l8};
  vector<map<string, Plaintext>> maps;
  vector<string> arrays = {"arr_0",  "arr_1",  "arr_2",  "arr_3", "arr_4",
                           "arr_5",  "arr_6",  "arr_7",  "arr_8", "arr_9",
                           "arr_10", "arr_11", "arr_12", "arr_13"};

  for (cnpy::npz_t learner : learners) {
    map<string, Plaintext> mappings;

    for (string idx : arrays) {
      cnpy::NpyArray arr = learner[idx];

      // cout << idx << ", " << arr.shape << endl;

      float* loaded_data = arr.data<float>();

      vector<float> curr;

      for (int i = 0; i < arr.shape[0]; i++) {
        curr.push_back(loaded_data[i]);
      }

      vector<complex<double>> curr2(curr.begin(), curr.end());

      Plaintext p = cc->MakeCKKSPackedPlaintext(curr2);

      mappings[idx] = p;
    }
    maps.push_back(mappings);
  }

  stop = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " milliseconds" << endl;

  cout << "(3) encrypting data & pwa algorithm!" << endl;

  start = high_resolution_clock::now();

  float weight = training_samples / (maps[0].size() * training_samples);

  Ciphertext<DCRTPoly> c = cc->Encrypt(kp1.publicKey, maps[0]["arr_0"]);

  auto pwa = cc->EvalMult(c, weight);

  for (string idx : arrays) {
    for (int i = 0; i < maps.size(); i++) {
      if (!(idx == ("arr_0")) && i != 0) {
        //cout << idx << ", " << i << endl;
        Ciphertext<DCRTPoly> ciphertext =
            cc->Encrypt(kp1.publicKey, maps[i][idx]);
        pwa += cc->EvalMult(ciphertext, weight);
      }
    }
  }

  stop = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " milliseconds" << endl;

  cout << "(4) decrypting and determining pwa!" << endl;

  start = high_resolution_clock::now();

  Plaintext decryptResult;
  cc->Decrypt(kp1.secretKey, pwa, &decryptResult);
  auto dr = decryptResult->GetCKKSPackedValue();
  //  cout << dr << endl;

  duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " milliseconds" << endl;

}