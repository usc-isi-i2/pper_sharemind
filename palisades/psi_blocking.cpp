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

deque<pair<tuple<string, string>, vector<int64_t>>> readFromCSVFile(
    string csvFilePath);

int longestRotationNeeded;

bool isMatch(CryptoContext<DCRTPoly> cc, LPKeyPair<DCRTPoly> keyPair,
             Plaintext recA, Plaintext recB, float threshold);

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

  vector<Plaintext> plaintextsA;
  vector<Plaintext> plaintextsB;

  deque<pair<tuple<string, string>, vector<int64_t>>>::iterator it;
  for (it = setA.begin(); it != setA.end(); it++) {
    plaintextsA.push_back(cc->MakePackedPlaintext(it->second));
  }

  deque<pair<tuple<string, string>, vector<int64_t>>>::iterator it2;
  for (it2 = setB.begin(); it2 != setB.end(); it2++) {
    plaintextsB.push_back(cc->MakePackedPlaintext(it2->second));
  }

  int psi = 0;
  float threshold = 0.5;

  for (int i = 0; i < plaintextsA.size(); i++) {
    for (int j = 0; j < plaintextsB.size(); j++) {
      cout << "Comparing records: " << i << "th record of set A, " << j
           << "th record of set B" << endl;
      bool er = isMatch(cc, keyPair, plaintextsA[i], plaintextsB[j], threshold);
      if (er) {
        psi += 1;
        cout << "Match found between: " << get<1>(setA[i]) << " and "
             << get<1>(setB[j]) << endl;
      }
    }
  }

  cout << "Total # of records with jaccard >= " << threshold << ": " << psi
       << endl;
}

bool isMatch(CryptoContext<DCRTPoly> cc, LPKeyPair<DCRTPoly> keyPair,
             Plaintext recA, Plaintext recB, float threshold) {

  auto start = std::chrono::high_resolution_clock::now();

  float overlap = 0;
  Ciphertext<DCRTPoly> cipherB = cc->Encrypt(keyPair.publicKey, recB);

  auto d = cc->EvalSub(recA, cipherB);
  Plaintext decryptResult;
  cc->Decrypt(keyPair.secretKey, d, &decryptResult);
  decryptResult->SetLength(recB->GetLength());

  overlap += count(decryptResult->GetPackedValue().begin(),
                   decryptResult->GetPackedValue().end(), 0);

  int idx = 1;
  vector<int64_t> a = recA->GetPackedValue();
  while (idx < a.size()) {
    std::rotate(a.begin(), a.begin() + 1, a.end());
    recA = cc->MakePackedPlaintext(a);

    auto d = cc->EvalSub(recA, cipherB);

    Plaintext decryptResult;
    cc->Decrypt(keyPair.secretKey, d, &decryptResult);
    decryptResult->SetLength(recB->GetLength());

    overlap += count(decryptResult->GetPackedValue().begin(),
                     decryptResult->GetPackedValue().end(), 0);

    idx++;
  }

  float jaccardSimilarity = (overlap) / ((recA->GetLength() + recB->GetLength()) - overlap);

  auto stop = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::seconds>(stop - start);
  cout << "time taken (s): " << duration.count() << " seconds" << endl;

  cout << "jaccard similarity score: " << jaccardSimilarity << endl;

  if (jaccardSimilarity >= threshold) {
    return true;
  } else {
    return false;
  }
  return false;
}

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
      //            cout << "hex value: " << intermediate << endl;
      uint64_t x;
      std::stringstream ss;
      ss << std::hex << intermediate;
      ss >> x;
      //            cout << "unsignted int value: " << x << endl;
      tokens.push_back(x);
    }

    records.push_back(make_pair(ids, tokens));
  }
  return records;
}