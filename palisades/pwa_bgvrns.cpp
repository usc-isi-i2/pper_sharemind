
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
//  cout << "(1) instantiating crypto-context" << endl;
//
  auto start = high_resolution_clock::now();

  int plaintextModulus = 65537;
  double sigma = 3.2;
  SecurityLevel securityLevel = HEStd_128_classic;
  uint32_t depth = 10;

  // Instantiate the BGVrns crypto context
  CryptoContext<DCRTPoly> cc =
      CryptoContextFactory<DCRTPoly>::genCryptoContextBGVrns(
          depth, plaintextModulus, securityLevel, sigma, depth, OPTIMIZED, BV);

  // enable features that you wish to use
  cc->Enable(ENCRYPTION);
  cc->Enable(SHE);
  cc->Enable(LEVELEDSHE);

  // Initialize Public Key Containers
  LPKeyPair<DCRTPoly> kp1;

  LPKeyPair<DCRTPoly> kpMultiparty;

  kp1 = cc->KeyGen();
  cc->EvalMultKeyGen(kp1.secretKey);

  auto stop = high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = stop - start;
//  cout << "time taken (s): " << duration.count() << " milliseconds" << endl;
//
//  cout << "(2) reading in .npz files & converting to plaintexts!" << endl;

//  start = high_resolution_clock::now();

  cout << "reading in learner 1!" << endl;
  cnpy::npz_t l1 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_0.npz");

  cout << "reading in learner 2!" << endl;
  cnpy::npz_t l2 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_1.npz");

  cout << "reading in learner 3!" << endl;
  cnpy::npz_t l3 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_2.npz");

  cout << "reading in learner 4!" << endl;
  cnpy::npz_t l4 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_3.npz");

  cout << "reading in learner 5!" << endl;
  cnpy::npz_t l5 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_4.npz");

  cout << "reading in learner 6!" << endl;
  cnpy::npz_t l6 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_5.npz");

  cout << "reading in learner 7!" << endl;
  cnpy::npz_t l7 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_6.npz");

  cout << "reading in learner 8!" << endl;
  cnpy::npz_t l8 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_7.npz");

  cout << "reading in learner 9!" << endl;
  cnpy::npz_t l9 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_8.npz");

  cout << "reading in learner 10!" << endl;
  cnpy::npz_t l10 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_9.npz");

  cout << "reading in learner 11!" << endl;
  cnpy::npz_t l11 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_10.npz");

  cout << "reading in learner 12!" << endl;
  cnpy::npz_t l12 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_11.npz");

  cout << "reading in learner 13!" << endl;
  cnpy::npz_t l13 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_12.npz");

  cout << "reading in learner 14!" << endl;
  cnpy::npz_t l14 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_13.npz");

  cout << "reading in learner 15!" << endl;
  cnpy::npz_t l15 = cnpy::npz_load(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/learners_flattened/learner1_14.npz");


  cout << "Benchmarking BGVrns.." << endl;

  //l1, l2, l3, l4, l5, l6, l7, l8, l9, l10, l11, l12, l13, l14, l15
  vector<cnpy::npz_t> learners = {l1, l2, l3, l4, l5};
  vector<map<string, Plaintext>> maps;
  vector<string> arrays = {"arr_0",  "arr_1",  "arr_2",  "arr_3", "arr_4",
                           "arr_5",  "arr_6",  "arr_7",  "arr_8", "arr_9",
                           "arr_10", "arr_11", "arr_12", "arr_13", "arr_14", "arr_15"};

  for (cnpy::npz_t learner : learners) {
    map<string, Plaintext> mappings;

    for (string idx : arrays) {
      cnpy::NpyArray arr = learner[idx];

      //cout << idx << ", " << arr.shape << endl;

      float* loaded_data = arr.data<float>();

      vector<float> curr;

      for (int i = 0; i < arr.shape[0]; i++) {
        curr.push_back(loaded_data[i]);
      }

      vector<int64_t> curr2;
      for (int i = 0; i < curr.size(); i++) {
        curr2[i] = (int64_t) curr[i];
      }

//      vector<int64_t> curr2(curr.begin(), curr.end());

//      cout << curr2 << endl;

      Plaintext p = cc->MakePackedPlaintext(curr2);

      mappings[idx] = p;
    }
    maps.push_back(mappings);
  }

//  stop = high_resolution_clock::now();
//  duration = duration_cast<milliseconds>(stop - start);
//  cout << "time taken (s): " << duration.count() << " milliseconds" << endl;

  cout << "Encrypting.." << endl;

  start = high_resolution_clock::now();

  map<int, vector<Ciphertext<DCRTPoly>>> ciphertexts;

  for (int i = 0; i < maps.size(); i++) {
    map<string, Plaintext> m = maps[i];
    map<string, Plaintext>::iterator it;

    vector<Ciphertext<DCRTPoly>> curr;

    for (it = m.begin(); it != m.end(); it++) {
      Ciphertext<DCRTPoly> ciphertext = cc->Encrypt(kp1.publicKey, it->second);
      curr.push_back(ciphertext);
    }
    ciphertexts[i] = curr;
  }

  stop = high_resolution_clock::now();
  duration = stop - start;
  cout << "Time taken by a learner to encrypt 1 million parameters: " << duration.count() / learners.size() << " milliseconds" << endl;

  cout << "Weighted Average.." << endl;

  start = high_resolution_clock::now();

  vector<Ciphertext<DCRTPoly>> c0 = ciphertexts[0];

  float training_samples = 1000;

  long weight = training_samples / (learners.size() * training_samples);

  auto w = cc->MakePackedPlaintext({weight});

  auto pwa = cc->EvalMult(w, c0[0]);

  for (int i = 0; i < arrays.size(); i++) {
    for (int j = 0; j < learners.size(); j++) {
      if (i == 0 && j == 0) {
      } else {
        //cout << i << ", " << j << endl;
        vector<Ciphertext<DCRTPoly>> c = ciphertexts[j];
        cc->EvalAdd(pwa, cc->EvalMult(w, c[i]));
      }
    }
  }

  vector<int64_t> denominator;
  denominator.push_back(learners.size());
  Plaintext a = cc->MakePackedPlaintext(denominator);
  Ciphertext<DCRTPoly> d = cc->Encrypt(kp1.publicKey, a);
  cc->EvalMult(pwa, d);

  stop = high_resolution_clock::now();
  duration = stop - start;
  cout << "Time taken to compute weighted average of 1 million encrypted parameters of 5 learners: " << duration.count() << " milliseconds" << endl;

  cout << "(4) Decrypting.." << endl;

  start = high_resolution_clock::now();

  Plaintext decryptResult;
  cc->Decrypt(kp1.secretKey, pwa, &decryptResult);
  auto dr = decryptResult->GetPackedValue();
  //  cout << dr << endl;

  duration = start - stop;
  cout << "time taken (s): " << duration.count() * 10 << " milliseconds" << endl;


}