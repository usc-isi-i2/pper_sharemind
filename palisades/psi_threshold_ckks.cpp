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
  LPKeyPair<DCRTPoly> kp2;
  LPKeyPair<DCRTPoly> kp3;

  LPKeyPair<DCRTPoly> kpMultiparty;

  kp1 = cc->KeyGen();
  cc->EvalMultKeyGen(kp1.secretKey);

  kp2 = cc->MultipartyKeyGen(kp1.publicKey);
  cc->EvalMultKeyGen(kp2.secretKey);

  kp3 = cc->MultipartyKeyGen(kp2.publicKey);
  cc->EvalMultKeyGen(kp3.secretKey);

  auto evalMultKey = cc->KeySwitchGen(kp1.secretKey, kp1.secretKey);

  auto evalMultKey2 =
      cc->MultiKeySwitchGen(kp2.secretKey, kp2.secretKey, evalMultKey);

  auto evalMultKey3 =
      cc->MultiKeySwitchGen(kp3.secretKey, kp3.secretKey, evalMultKey2);

  auto ab = cc->MultiAddEvalKeys(evalMultKey, evalMultKey2);
  auto evalMultABC =
      cc->MultiAddEvalKeys(ab, evalMultKey3, kp3.publicKey->GetKeyTag());

  auto evalMultCABC = cc->MultiMultEvalKey(evalMultABC, kp3.secretKey,
                                           kp3.publicKey->GetKeyTag());

  auto evalMultBABC = cc->MultiMultEvalKey(evalMultABC, kp2.secretKey,
                                           kp3.publicKey->GetKeyTag());

  auto evalMultAABC = cc->MultiMultEvalKey(evalMultABC, kp1.secretKey,
                                           kp3.publicKey->GetKeyTag());

  auto final_ab =
      cc->MultiAddEvalMultKeys(evalMultAABC, evalMultBABC, ab->GetKeyTag());

  auto final_abc = cc->MultiAddEvalMultKeys(evalMultCABC, final_ab,
                                            evalMultABC->GetKeyTag());

  cc->InsertEvalMultKey({final_abc});

  //
  //  std::cout
  //      << "Joint evaluation multiplication key for (s_a + s_b) is
  //      generated..."
  //      << std::endl;
  //  auto evalMultAB = cc->MultiAddEvalKeys(evalMultKey, evalMultKey2,
  //                                         kp2.publicKey->GetKeyTag());
  //
  //  std::cout << "Joint evaluation multiplication key (s_a + s_b) is
  //  transformed "
  //               "into s_b*(s_a + s_b)..."
  //            << std::endl;
  //  auto evalMultBAB = cc->MultiMultEvalKey(evalMultAB, kp2.secretKey,
  //                                          kp2.publicKey->GetKeyTag());
  //
  //
  //  std::cout << "Joint key (s_a + s_b) is transformed into s_a*(s_a +
  //  s_b)..."
  //            << std::endl;
  //  auto evalMultAAB = cc->MultiMultEvalKey(evalMultAB, kp1.secretKey,
  //                                          kp2.publicKey->GetKeyTag());
  //
  //  std::cout << "Computing the final evaluation multiplication key for (s_a +
  //  "
  //               "s_b)*(s_a + s_b)..."
  //            << std::endl;
  //  auto evalMultFinal = cc->MultiAddEvalMultKeys(evalMultAAB, evalMultBAB,
  //                                                evalMultAB->GetKeyTag());
  //
  //  std::cout << "Round 3 of key generation completed." << std::endl;
  //
  //  cc->InsertEvalMultKey({evalMultFinal});

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
        cc->Encrypt(kp3.publicKey, plaintextsB[i]);
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

    auto ciphertextPartial3 =
        cc->MultipartyDecryptMain(kp3.secretKey, {returnCiphertexts[i]});

    vector<Ciphertext<DCRTPoly>> partialCiphertextVecMult;
    partialCiphertextVecMult.push_back(ciphertextPartial1[0]);
    partialCiphertextVecMult.push_back(ciphertextPartial2[0]);
    partialCiphertextVecMult.push_back(ciphertextPartial3[0]);

    cc->MultipartyDecryptFusion(partialCiphertextVecMult,
                                &plaintextMultipartyMult);

    plaintextMultipartyMult->SetLength(plaintextsB[0]->GetLength());

    cout << plaintextMultipartyMult << endl;

    auto v = plaintextMultipartyMult->GetCKKSPackedValue();

    if (std::all_of(v.cbegin(), v.cend(),
                    [](complex<double> i) { return i.real() < 0.001; })) {
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