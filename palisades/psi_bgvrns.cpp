#include "palisade.h"
#include <iostream>
#include <fstream>
#include <map>
#include <time.h>
#include <stdlib.h>
#include "csvstream.h"
#include <sstream>

using namespace std;
using namespace lbcrypto;

deque<pair<string, vector<int64_t>>> readFromCSVFile(string csvFilePath);

int main() {

  int plaintextModulus = 65537;
  double sigma = 3.2;
  SecurityLevel securityLevel = HEStd_128_classic;
  uint32_t depth = 10;

  // Instantiate the BGVrns crypto context
  CryptoContext<DCRTPoly> cc =
      CryptoContextFactory<DCRTPoly>::genCryptoContextBGVrns(
          depth, plaintextModulus, securityLevel, sigma, depth, OPTIMIZED, BV);

  // Enable features that to use
  cc->Enable(ENCRYPTION);
  cc->Enable(SHE);
  cc->Enable(MULTIPARTY);
  // Initialize Public Key Containers
  LPKeyPair<DCRTPoly> keyPair;

  // Generate a public/private key pair
  keyPair = cc->KeyGen();

  cc->EvalMultKeyGen(keyPair.secretKey);

  cout << "reading in files!" << endl;

  deque<pair<string, vector<int64_t>>> setA = readFromCSVFile(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/tokenized_set_A.csv");
  deque<pair<string, vector<int64_t>>> setB = readFromCSVFile(
      "/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/"
      "test_data/tokenized_set_B.csv");

  vector<Plaintext> plaintextsA;
  vector<Plaintext> plaintextsB;

  cout << "converting to plaintexts!" << endl;

  deque<pair<string, vector<int64_t>>>::iterator it;
  for (it = setA.begin(); it != setA.end(); it++) {
    plaintextsA.push_back(cc->MakePackedPlaintext(it->second));
  }

  deque<pair<string, vector<int64_t>>>::iterator it2;
  for (it2 = setB.begin(); it2 != setB.end(); it2++) {
    plaintextsB.push_back(cc->MakePackedPlaintext(it2->second));
  }

  cout << "encrypting set B!" << endl;

  vector<Ciphertext<DCRTPoly>> ciphertexts;
  for (int i = 0; i < plaintextsB.size(); i++) {
    Ciphertext<DCRTPoly> ciphertext =
        cc->Encrypt(keyPair.publicKey, plaintextsB[i]);
    ciphertexts.push_back(ciphertext);
  }

  cout << "running PSI algorithm!" << endl;

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

  cout << "decrypting and determining set intersection size!" << endl;

  double setIntersectionSize = 0;
  for (int i = 0; i < returnCiphertexts.size(); i++) {
    Plaintext decryptResult;
    cc->Decrypt(keyPair.secretKey, returnCiphertexts[i], &decryptResult);
    vector<int64_t> v(decryptResult->GetPackedValue().size(), 0);
    if (decryptResult->GetPackedValue() == v) {
      setIntersectionSize++;
    }
  }
  cout << "set intersection size: " << setIntersectionSize << endl;
  cout << "denominator: "
       << ((plaintextsA.size() + plaintextsB.size()) - setIntersectionSize)
       << endl;
  double jaccard =
      setIntersectionSize /
      ((plaintextsA.size() + plaintextsB.size()) - setIntersectionSize);
  printf("%f\n", jaccard);
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