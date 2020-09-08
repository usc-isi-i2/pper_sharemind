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

using namespace std;
using namespace lbcrypto;

vector<vector<string>> readFromCSVFile(string csvFilePath);
vector<string> tokenHelper(int n, string record);
deque<pair<string, vector<string>>> tokenizeRecords(vector<vector<string>> set);
vector<Plaintext> preProcessPlaintexts(CryptoContext<DCRTPoly> cc,
                                       deque<pair<string, vector<string>>> set);
vector<int64_t> simpleIntegerPSI(CryptoContext<DCRTPoly> cc,
                                 LPKeyPair<DCRTPoly> keyPair,
                                 vector<int64_t> setX, vector<int64_t> setY);

int main() {
  int plaintextModulus = 65537;
  double sigma = 3.2;
  SecurityLevel securityLevel = HEStd_128_classic;
  uint32_t depth = 2;

  // Instantiate the BGVrns crypto context
  CryptoContext<DCRTPoly> cc =
      CryptoContextFactory<DCRTPoly>::genCryptoContextBGVrns(
          depth, plaintextModulus, securityLevel, sigma, depth, OPTIMIZED, BV);

  // Instantiate the BFVrns crypto context
  //  CryptoContext<DCRTPoly> cc =
  //      CryptoContextFactory<DCRTPoly>::genCryptoContextBFVrns(
  //          plaintextModulus, securityLevel, sigma, 0, depth, 0, OPTIMIZED);

  // Enable features that to use
  cc->Enable(ENCRYPTION);
  cc->Enable(SHE);
  cc->Enable(MULTIPARTY);
  // Initialize Public Key Containers
  LPKeyPair<DCRTPoly> keyPair;

  // Generate a public/private key pair
  keyPair = cc->KeyGen();

  cc->EvalMultKeyGen(keyPair.secretKey);

    vector<vector<string>> setA = readFromCSVFile(
        "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
        "test1.csv");
    vector<vector<string>> setB = readFromCSVFile(
        "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
        "test2.csv");

    deque<pair<string, vector<string>>> a = tokenizeRecords(setA);
    deque<pair<string, vector<string>>> b = tokenizeRecords(setB);


    vector<Plaintext> plaintextsA = preProcessPlaintexts(cc, a);
    vector<Plaintext> plaintextsB = preProcessPlaintexts(cc, b);

  //  vector<int64_t> setX = {3, 2, 1, 4};
  //  vector<int64_t> setY = {1, 2, 3, 5};
  //  cout << "set X: " << setX << endl;
  //  cout << "set Y: " << setY << endl;
  //
  //  cout << "Set Intersection between sets X and Y: "
  //       << simpleIntegerPSI(cc, keyPair, setX, setY) << endl;

    vector<Ciphertext<DCRTPoly>> ciphertexts;
    for (int i = 0; i < plaintextsB.size(); i++) {
      Ciphertext<DCRTPoly> ciphertext =
          cc->Encrypt(keyPair.publicKey, plaintextsB[i]);
      ciphertexts.push_back(ciphertext);
    }

    vector<Ciphertext<DCRTPoly>> returnCiphertexts;
    for (int i = 0; i < ciphertexts.size(); i++) {
//      vector<int64_t> v(8192);
//      generate(v.begin(), v.end(), RandomGenerator(100));
//      Plaintext r = cc->MakePackedPlaintext(v);
      auto sub1 = cc->EvalSub(ciphertexts[i], plaintextsA[0]);
      auto sub2 = cc->EvalSub(ciphertexts[i], plaintextsA[1]);
      auto d = cc->EvalMult(sub1, sub2);
      for (int j = 2; j < plaintextsA.size(); j++) {
        d = cc->EvalMult(d, cc->EvalSub(ciphertexts[i], plaintextsA[j]));
      }
      returnCiphertexts.push_back(d);
    }

    vector<string> setIntersectionResult;
    for (int i = 0; i < returnCiphertexts.size(); i++) {
      Plaintext decryptResult;
      cc->Decrypt(keyPair.secretKey, returnCiphertexts[i], &decryptResult);
      vector<int64_t> v(decryptResult->GetPackedValue().size(), 0);
      if (decryptResult->GetPackedValue() == v) {
        setIntersectionResult.push_back(b[i].first);
      }
    }
    cout << "Records in Set Intersection: " << setIntersectionResult << endl;

  return 0;
}

vector<int64_t> simpleIntegerPSI(CryptoContext<DCRTPoly> cc,
                                 LPKeyPair<DCRTPoly> keyPair,
                                 vector<int64_t> setX, vector<int64_t> setY) {
  vector<Plaintext> pX;
  vector<Plaintext> pY;

  // generating simple integer plaintexts for setX
  for (int i = 0; i < setX.size(); i++) {
    pX.push_back(cc->MakeIntegerPlaintext(setX[i]));
  }

  // generating simple integer plaintexts for setY
  for (int i = 0; i < setY.size(); i++) {
    pY.push_back(cc->MakeIntegerPlaintext(setY[i]));
  }

  // encrypting contents of set Y
  vector<Ciphertext<DCRTPoly>> ciphertexts;
  for (int i = 0; i < pY.size(); i++) {
    ciphertexts.push_back(cc->Encrypt(keyPair.publicKey, pY[i]));
  }

  // implementing Basic PSI protocol from: https://eprint.iacr.org/2017/299.pdf
  vector<Ciphertext<DCRTPoly>> returnCiphertexts;
  for (int i = 0; i < ciphertexts.size(); i++) {
    // sample random non-zero plaintext
    Plaintext r = cc->MakeIntegerPlaintext((rand() % 100) + 1);
    // initializing d_i to be product of difference between each c_i and each x
    // in X
    auto sub1 = cc->EvalSub(ciphertexts[i], pX[0]);
    auto sub2 = cc->EvalSub(ciphertexts[i], pX[1]);
    auto d = cc->EvalMult(sub1, sub2);
    for (int j = 2; j < pX.size(); j++) {
      d = cc->EvalMult(d, cc->EvalSub(ciphertexts[i], pX[j]));
    }
    returnCiphertexts.push_back(cc->EvalMult(r, d));
  }

  vector<int64_t> setIntersectionResult;
  for (int i = 0; i < returnCiphertexts.size(); i++) {
    Plaintext decryptResult;
    // decrypting the ciphertexts; if y+i : FHE.decrypt(d_i) = 0, then we know
    // that y_i exists in X and is part of the set intersection.
    cc->Decrypt(keyPair.secretKey, returnCiphertexts[i], &decryptResult);
    if (decryptResult->GetIntegerValue() == 0) {
      setIntersectionResult.push_back(setY[i]);
    }
  }

  return setIntersectionResult;
}

/*
 * preProcessPlaintexts(map<string, vector<string>>): convert each vector of
 * n-grams into a vector of int64_t's to prepare for encryption with Palisades
 * functions.
 */
vector<Plaintext> preProcessPlaintexts(CryptoContext<DCRTPoly> cc,
                                            deque<pair<string, vector<string>>> set) {
  vector<Plaintext> plaintexts;
  deque<pair<string, vector<string>>>::iterator it;
  for (it = set.begin(); it != set.end(); it++) {
    vector<int64_t> plaintext;
    vector<string> v = it->second;
    for (string s : v) {
      for (char c : s) {
        plaintext.push_back(int(c));
      }
    }
    plaintexts.push_back(cc->MakePackedPlaintext(plaintext));
  }
  return plaintexts;
}

/*
 * tokenHelper(int n, string record): helper method for the tokenization
 * process, converting a string to a n-gram, with n being a parameter.
 */
vector<string> tokenHelper(int n, string record) {
  vector<string> tokenized;
  if (record.size() == 0) {
    return tokenized;
  }

  for (int i = 0; i < record.size() - n + 1; i++) {
    tokenized.push_back(record.substr(i, n));
  }
  return tokenized;
}

/*
 * tokenizeRecords(vector<vector<string>> set): converts each vector of strings
 * into a tokenized set of n-grams. For my purposes here, I chose bi-grams.
 * ex: ["tanmay"] -> ["ta", "an", "nm", "ma", ay"]
 */
deque<pair<string, vector<string>>> tokenizeRecords(vector<vector<string>> set) {
  deque<pair<string, vector<string>>> tokenSet;
  for (int i = 0; i < set.size(); i++) {
    string key = set[i][set[i].size() - 1];
    set[i].erase(set[i].begin());
    set[i].erase(set[i].end() - 1);

    string s;
    for (const auto &piece : set[i]) s += piece;
    tokenSet.push_back(make_pair(key, tokenHelper(2, s)));
  }
  return tokenSet;
}
/*
 * readFromCSVFile(string csvFilePath): reads in the two csv files, each
 * second-level vector is a single line in the csv file where each string in the
 * vector is a comma separated field in the input.
 */
vector<vector<string>> readFromCSVFile(string csvFilePath) {
  csvstream csvin(csvFilePath);

  vector<vector<string>> records;
  vector<pair<string, string>> row;

  while (csvin >> row) {
    vector<string> record;
    for (unsigned int i = 0; i < row.size(); ++i) {
      record.push_back(row[i].second);
    }
    records.push_back(record);
  }
  return records;
}
