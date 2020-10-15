// @file  simple-integers.cpp - Simple example for BFVrns (integer arithmetic).
// @author TPOC: contact@palisade-crypto.org
//
// @copyright Copyright (c) 2019, New Jersey Institute of Technology (NJIT))
// All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution. THIS SOFTWARE IS
// PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "palisade.h"
#include <iostream>
#include <fstream>
#include <map>
#include <time.h>
#include <stdlib.h>
#include "csvstream.h"
#include <Python.h>
#include <sstream>
#include <chrono>

using namespace std;
using namespace lbcrypto;
using namespace std::chrono;

deque<pair<string, vector<int64_t>>> readFromCSVFile(string csvFilePath);

int main() {
  Py_Initialize();
  PyObject *obj =
      Py_BuildValue("s",
                    "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/"
                    "examples/data_processing.py");
  FILE *file = _Py_fopen_obj(obj, "r+");
  if (file != NULL) {
    PyRun_SimpleFile(file, "data_processing.py");
  }
  Py_Finalize();

  cout << "(1) reading in files!" << endl;

  auto start = high_resolution_clock::now();

  deque<pair<string, vector<int64_t>>> setA = readFromCSVFile(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/tokenized_set_A.csv");
  deque<pair<string, vector<int64_t>>> setB = readFromCSVFile(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/tokenized_set_B.csv");

  vector<Plaintext> plaintextsA;
  vector<Plaintext> plaintextsB;

  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<seconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " seconds" << endl;

  cout << "(2) converting to plaintexts!" << endl;

  start = high_resolution_clock::now();

  int depth;
  if (setA.size() >= setB.size()) {
    depth = setA.size();
  } else {
    depth = setB.size();
  }
  int plaintextModulus = (depth * 131072) + 1;
  double sigma = 3.2;
  SecurityLevel securityLevel = HEStd_128_classic;

  EncodingParams encodingParams(new EncodingParamsImpl(plaintextModulus));

  usint batchSize = 1024;
  encodingParams->SetBatchSize(batchSize);

  // Instantiate the crypto context
  CryptoContext<DCRTPoly> cc =
      CryptoContextFactory<DCRTPoly>::genCryptoContextBGVrns(
          depth, plaintextModulus, securityLevel, sigma, depth, OPTIMIZED, BV);


  uint32_t m = cc->GetCyclotomicOrder();
  PackedEncoding::SetParams(m, encodingParams);

  // enable features that you wish to use
  cc->Enable(ENCRYPTION);
  cc->Enable(SHE);
  cc->Enable(MULTIPARTY);

  // Initialize Public Key Containers
  LPKeyPair<DCRTPoly> kp1;
  LPKeyPair<DCRTPoly> kp2;

  LPKeyPair<DCRTPoly> kpMultiparty;

  kp1 = cc->KeyGen();
  cc->EvalMultKeyGen(kp1.secretKey);

  auto evalMultKey = cc->KeySwitchGen(kp1.secretKey, kp1.secretKey);

  kp2 = cc->MultipartyKeyGen(kp1.publicKey);
  cc->EvalMultKeyGen(kp2.secretKey);

  auto evalMultKey2 =
      cc->MultiKeySwitchGen(kp2.secretKey, kp2.secretKey, evalMultKey);

  std::cout
      << "Joint evaluation multiplication key for (s_a + s_b) is generated..."
      << std::endl;
  auto evalMultAB = cc->MultiAddEvalKeys(evalMultKey, evalMultKey2,
                                         kp2.publicKey->GetKeyTag());

  std::cout << "Joint evaluation multiplication key (s_a + s_b) is transformed "
               "into s_b*(s_a + s_b)..."
            << std::endl;
  auto evalMultBAB = cc->MultiMultEvalKey(evalMultAB, kp2.secretKey,
                                          kp2.publicKey->GetKeyTag());


  std::cout << "Joint key (s_a + s_b) is transformed into s_a*(s_a + s_b)..."
            << std::endl;
  auto evalMultAAB = cc->MultiMultEvalKey(evalMultAB, kp1.secretKey,
                                          kp2.publicKey->GetKeyTag());

  std::cout << "Computing the final evaluation multiplication key for (s_a + "
               "s_b)*(s_a + s_b)..."
            << std::endl;
  auto evalMultFinal = cc->MultiAddEvalMultKeys(evalMultAAB, evalMultBAB,
                                                evalMultAB->GetKeyTag());

  cc->InsertEvalMultKey({evalMultFinal});

  std::cout << "Round 3 of key generation completed." << std::endl;

  deque<pair<string, vector<int64_t>>>::iterator it;
  for (it = setA.begin(); it != setA.end(); it++) {
    plaintextsA.push_back(cc->MakePackedPlaintext(it->second));
  }

  deque<pair<string, vector<int64_t>>>::iterator it2;
  for (it2 = setB.begin(); it2 != setB.end(); it2++) {
    plaintextsB.push_back(cc->MakePackedPlaintext(it2->second));
  }

  stop = high_resolution_clock::now();
  duration = duration_cast<seconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " seconds" << endl;

  cout << "(3) encrypting set B!" << endl;

  start = high_resolution_clock::now();

  vector<Ciphertext<DCRTPoly>> ciphertexts;
  for (int i = 0; i < plaintextsB.size(); i++) {
    Ciphertext<DCRTPoly> ciphertext =
        cc->Encrypt(kp2.publicKey, plaintextsB[i]);
    ciphertexts.push_back(ciphertext);
  }

  stop = high_resolution_clock::now();
  duration = duration_cast<seconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " seconds" << endl;

  cout << "(4) running PSI algorithm!" << endl;

  start = high_resolution_clock::now();

  double setIntersectionSize = 0;

  vector<Ciphertext<DCRTPoly>> returnCiphertexts;
    for (int i = 0; i < ciphertexts.size(); i++) {
      auto sub1 = cc->EvalSub(ciphertexts[i], plaintextsA[0]);
      auto sub2 = cc->EvalSub(ciphertexts[i], plaintextsA[1]);
      auto d = cc->EvalMult(sub1, sub2);
      for (int j = 2; j < plaintextsA.size(); j++) {
        d = cc->EvalMult(d, cc->EvalSub(ciphertexts[i], plaintextsA[j]));
      }
      returnCiphertexts.push_back(d);
    }

  stop = high_resolution_clock::now();
  duration = duration_cast<seconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " seconds" << endl;

  cout << "(5) decrypting and determining set intersection size!" << endl;

  start = high_resolution_clock::now();

  const shared_ptr<LPCryptoParameters<DCRTPoly>> cryptoParams =
      kp1.secretKey->GetCryptoParameters();
  const shared_ptr<typename DCRTPoly::Params> elementParams =
      cryptoParams->GetElementParams();

  for (int i = 0; i < returnCiphertexts.size(); i++) {
    Plaintext plaintextMultipartyMult;

    auto ciphertextPartial1 =
        cc->MultipartyDecryptLead(kp1.secretKey, {returnCiphertexts[i]});

    auto ciphertextPartial2 =
        cc->MultipartyDecryptMain(kp2.secretKey, {returnCiphertexts[i]});

    vector<Ciphertext<DCRTPoly>> partialCiphertextVecMult;
    partialCiphertextVecMult.push_back(ciphertextPartial1[0]);
    partialCiphertextVecMult.push_back(ciphertextPartial2[0]);

    cc->MultipartyDecryptFusion(partialCiphertextVecMult,
                                &plaintextMultipartyMult);

    plaintextMultipartyMult->SetLength(plaintextsB[0]->GetLength());
    vector<int64_t> v(plaintextMultipartyMult->GetPackedValue().size(), 0);
      if (plaintextMultipartyMult->GetPackedValue() == v) {
        setIntersectionSize++;
      }
  }

    stop = high_resolution_clock::now();

  duration = duration_cast<seconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " seconds" << endl;

  cout << "set intersection size: " << setIntersectionSize << endl;

  double jaccard =
      setIntersectionSize /
      ((plaintextsA.size() + plaintextsB.size()) - setIntersectionSize);
  cout << "jaccard similarity score: ";
  printf("%lf\n", jaccard);

  return 0;
}

/*
 * readFromCSVFile(string csvFilePath): reads in the two csv files, each
 * second-level vector is a single line in the csv file where each string in the
 * vector is a comma separated field in the input.
 */
deque<pair<string, vector<int64_t>>> readFromCSVFile(string csvFilePath) {
  csvstream csvin(csvFilePath, ':', false);

  deque<pair<string, vector<int64_t>>> records;
  vector<pair<string, string>> row;

  while (csvin >> row) {
    vector<int64_t> record;
    string key;
    for (unsigned int i = 0; i < row.size(); ++i) {
      stringstream s_stream(row[i].second);
      while (s_stream.good()) {
        string substr;
        getline(s_stream, substr, ',');
        if (i == 1) {
          substr = substr.substr(1);
        } else if (i == row.size() - 1) {
          substr = substr.substr(0, row.size() - 2);
        }
        if (i == 0) {
          key = substr;
        } else {
          record.push_back(stoi(substr));
        }
      }
    }
    records.push_back(make_pair(key, record));
  }
  return records;
}
