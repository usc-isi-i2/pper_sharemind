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

deque<pair<string, vector<complex<double>>>> readFromCSVFile(
    string csvFilePath);

int main() {
  cout << "(1) reading in files!" << endl;

  auto start = high_resolution_clock::now();

  deque<pair<string, vector<complex<double>>>> setA = readFromCSVFile(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/tokenized_set_A.csv");
  deque<pair<string, vector<complex<double>>>> setB = readFromCSVFile(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/tokenized_set_B.csv");

  vector<Plaintext> plaintextsA;
  vector<Plaintext> plaintextsB;

  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<seconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " seconds" << endl;

  cout << "(2) converting to plaintexts!" << endl;

  start = high_resolution_clock::now();

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


  deque<pair<string, vector<complex<double>>>>::iterator it;
  for (it = setA.begin(); it != setA.end(); it++) {
    plaintextsA.push_back(cc->MakeCKKSPackedPlaintext(it->second));
  }

  deque<pair<string, vector<complex<double>>>>::iterator it2;
  for (it2 = setB.begin(); it2 != setB.end(); it2++) {
    plaintextsB.push_back(cc->MakeCKKSPackedPlaintext(it2->second));
  }

  stop = high_resolution_clock::now();
  duration = duration_cast<seconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " seconds" << endl;

  cout << "(3) encrypting set B!" << endl;

  start = high_resolution_clock::now();

  vector<Ciphertext<DCRTPoly>> ciphertexts;
  for (int i = 0; i < plaintextsB.size(); i++) {
    Ciphertext<DCRTPoly> ciphertext =
        cc->Encrypt(kp1.publicKey, plaintextsB[i]);
    ciphertexts.push_back(ciphertext);
  }

  stop = high_resolution_clock::now();
  duration = duration_cast<seconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " seconds" << endl;

  cout << "(4) running PSI algorithm!" << endl;

  start = high_resolution_clock::now();

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

  double setIntersectionSize = 0;
  for (int i = 0; i < returnCiphertexts.size(); i++) {
    Plaintext decryptResult;
    cc->Decrypt(kp1.secretKey, returnCiphertexts[i], &decryptResult);
    vector<complex<double>> v(decryptResult->GetCKKSPackedValue().size(), 0);
    if (decryptResult->GetCKKSPackedValue() == v) {
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
deque<pair<string, vector<complex<double>>>> readFromCSVFile(
    string csvFilePath) {
  csvstream csvin(csvFilePath, ':', false);

  deque<pair<string, vector<complex<double>>>> records;
  vector<pair<string, string>> row;

  while (csvin >> row) {
    vector<complex<double>> record;
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