
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

  CryptoContext<DCRTPoly> cc =
      CryptoContextFactory<DCRTPoly>::genCryptoContextBGVrns(
          2, 65537, securityLevel);

  // enable features that you wish to use
  cc->Enable(ENCRYPTION);
  cc->Enable(SHE);
  cc->Enable(LEVELEDSHE);

  LPKeyPair<DCRTPoly> keyPair;

  keyPair = cc->KeyGen();
  cc->EvalMultKeyGen(keyPair.secretKey);

  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " ms" << endl;

  cout << "(2) converting to plaintexts!" << endl;

  start = high_resolution_clock::now();


  // weights
  vector<double> weights = {30, 20, 10, 40};

  // user input
  vector<int64_t> input = {1, 2, 3, 5};

  vector<Plaintext> p_weights;
  vector<Plaintext> p_input;

  for (int i = 0; i < weights.size(); i++) {
    p_weights.push_back(cc->MakeIntegerPlaintext(weights[i]));
  }

  for (int i = 0; i < input.size(); i++) {
    p_input.push_back(cc->MakeIntegerPlaintext(input[i]));
  }

  stop = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " ms" << endl;

  cout << "(2) encrypting weights and input data!" << endl;

  start = high_resolution_clock::now();

  vector<Ciphertext<DCRTPoly>> c_weights;
  for (int i = 0; i < p_weights.size(); i++) {
    c_weights.push_back(cc->Encrypt(keyPair.publicKey, p_weights[i]));
  }

  vector<Ciphertext<DCRTPoly>> c_input;
  for (int i = 0; i < p_input.size(); i++) {
    c_input.push_back(cc->Encrypt(keyPair.publicKey, p_input[i]));
  }

  stop = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " ms" << endl;

  cout << "(3) calculating homomorphic weighted average!" << endl;

  start = high_resolution_clock::now();

  auto sum = cc->EvalMult(c_weights[0], c_input[0]);
  for (int i = 1; i < weights.size(); i++) {
    auto intermediate_val = cc->EvalMult(c_weights[i], c_input[i]);
    sum = cc->EvalAdd(sum, intermediate_val);
  }

  stop = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " ms" << endl;

  cout << "(4) decrypting result!" << endl;

  Plaintext decryptResult;
  cc->Decrypt(keyPair.secretKey, sum, &decryptResult);

  cout << decryptResult << endl;



}
