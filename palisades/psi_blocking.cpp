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

deque<pair<tuple<string, string>, vector<int64_t>>> readFromCSVFile(
    string csvFilePath);

bool isMatch(CryptoContext<DCRTPoly> cc, LPKeyPair<DCRTPoly> keyPair, vector<Plaintext> recA, vector<Ciphertext<DCRTPoly>> recB, float threshold);

int main(int argc, char** argv) {
  string path = std::__fs::filesystem::current_path();

  string ds1 = argv[1];
  string ds2 = argv[2];
  // bool isBlockingEnabled = argv[3];

  cout << ds1 << endl;
  cout << ds2 << endl;

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
  // Initialize Public Key Containers
  LPKeyPair<DCRTPoly> keyPair;

  // Generate a public/private key pair
  keyPair = cc->KeyGen();

  cc->EvalMultKeyGen(keyPair.secretKey);

  deque<pair<tuple<string, string>, vector<int64_t>>> setA =
      readFromCSVFile(path + "/../src/pke/examples/test_data/" + ds1);
  deque<pair<tuple<string, string>, vector<int64_t>>> setB =
      readFromCSVFile(path + "/../src/pke/examples/test_data/" + ds2);

  vector<vector<Plaintext>> plaintextsA;
  vector<vector<Plaintext>> plaintextsB;

  deque<pair<tuple<string, string>, vector<int64_t>>>::iterator it;
  for (it = setA.begin(); it != setA.end(); it++) {
    vector<Plaintext> pA;
    cout << get<1>(it->first) << endl;
    for (int i = 0; i < it->second.size(); i++) {
      pA.push_back(cc->MakeIntegerPlaintext(it->second[i]));
    }
    plaintextsA.push_back(pA);
  }

  cout << "FINISHED SET A" << endl;

  deque<pair<tuple<string, string>, vector<int64_t>>>::iterator it2;
  for (it2 = setB.begin(); it2 != setB.end(); it2++) {
    vector<Plaintext> pB;
    cout << get<1>(it2->first) << endl;
    for (int i = 0; i < it2->second.size(); i++) {
      pB.push_back(cc->MakeIntegerPlaintext(it2->second[i]));
    }
    plaintextsB.push_back(pB);
  }


  vector<Plaintext> recA = plaintextsA[0];
  vector<Ciphertext<DCRTPoly>> recB;

  for (int i = 0; i < plaintextsB[0].size(); i++) {
    recB.push_back(cc->Encrypt(keyPair.publicKey, plaintextsB[0][i]));
  }

  cout << recA << endl;
  cout << recB << endl;

//  cout << isMatch(cc, keyPair, recA,  recB, 1) << endl;


  return 0;
}

bool isMatch(CryptoContext<DCRTPoly> cc, LPKeyPair<DCRTPoly> keyPair, vector<Plaintext> recA, vector<Ciphertext<DCRTPoly>> recB, float threshold) {
//  int setIntersectionSize = 0;
  for (int i = 0; i < recB.size(); i++) {
    auto sub1 = cc->EvalSub(recB[i], recA[0]);
    auto sub2 = cc->EvalSub(recB[i], recA[1]);
    auto d = cc->EvalMult(sub1, sub2);
    for (int j = 2; j < recA.size(); j++) {
      d = cc->EvalMult(d, cc->EvalSub(recB[i], recA[j]));
    }

//    Plaintext decryptResult;
//    cc->Decrypt(keyPair.secretKey, d, &decryptResult);
//    vector<int64_t> v(decryptResult->GetPackedValue().size(), 0);
//    if (decryptResult->GetPackedValue() == v) {
//      setIntersectionSize++;
//    }
  }

//  cout << setIntersectionSize << endl;
  return false;

}

/*
 * readFromCSVFile(string csvFilePath): reads in the two csv files, each
 * second-level vector is a single line in the csv file where each string in the
 * vector is a comma separated field in the input.
 */
deque<pair<tuple<string, string>, vector<int64_t>>> readFromCSVFile(
    string csvFilePath) {
  csvstream csvin(csvFilePath);

  deque<pair<tuple<string, string>, vector<int64_t>>> records;
  map<string, string> row;

  while (csvin >> row) {
    tuple<string, string> ids{row["id"], row["original_id"]};

    vector<int64_t> tokens;

    stringstream t(row["tokens"]);

    string intermediate;

    while (getline(t, intermediate, ' ')) {
      tokens.push_back(stoi(intermediate.substr(2)));
    }
    records.push_back(make_pair(ids, tokens));
  }
  return records;
}