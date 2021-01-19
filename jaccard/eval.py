import os
import csv
import sys
import argparse
from collections import defaultdict


EPSILON = 0.00001;

def extract_ori_rec_id(s):
    return '-'.join(s.split('-')[:2])

def convert_tokens(ts):
    return set([int(t, 16) for t in ts])

def jaccard(a, b):
    return 1.0 * len(a & b) / len(a | b)

def jaccard_sort(a, b):
    c = list(a) + list(b)
    c = sorted(c)
    inter = sum([int(i == j) for i, j in zip(c[:-1], c[1:])])
    union = len(a) + len(b)
    print(inter, union)
    return 1.0 * inter / (union - inter)

def compute_metrics(num_of_blocked_comparison, num_full_pairwise_comparison, 
                    num_of_match_in_block, num_of_match):
    rr = 1 - 1.0 * num_of_blocked_comparison / num_full_pairwise_comparison
    pc = 1.0 * num_of_match_in_block / num_of_match
    f1 = 2.0 * pc * rr / (pc + rr)
    return round(pc, 2), round(rr, 2), round(f1, 2)

if __name__ == '__main__':
    '''
    python eval.py test_data/ds1_output_0.8.csv test_data/ds2_output_0.8.csv --blocking --threshold 0.8
    '''

    parser = argparse.ArgumentParser(description='Evaluate Febrl dataset')
    parser.add_argument('infile1', nargs='?', type=argparse.FileType('r'))
    parser.add_argument('infile2', nargs='?', type=argparse.FileType('r'))
    parser.add_argument('--blocking', dest='blocking', action='store_true')
    parser.add_argument('--threshold', dest='threshold', action='store', type=float)
    parser.add_argument('-v', '--verbose', dest='verbose', action='store_true')
    args = parser.parse_args()

    fieldnames = ['id', 'original_id', 'tokens'] + ['blocking_keys'] if args.blocking else []
    csv1 = csv.DictReader(args.infile1)
    csv2 = csv.DictReader(args.infile2)

    bkey_per_record = 0
    ds1_ori_id_to_rec_id, ds2_ori_id_to_rec_id = defaultdict(set), defaultdict(set)
    ds1_ii, ds2_ii = defaultdict(set), defaultdict(set)
    ds1_size, ds2_size = 0, 0
    ds1, ds2 = {}, {}
    for idx, line in enumerate(csv1):
        if bkey_per_record == 0:
            bkey_per_record = len(line['blocking_keys'].split(' '))

        line['id'] = int(line['id'])
        ds1_size += 1
        ds1_ori_id_to_rec_id[extract_ori_rec_id(line['original_id'])].add(line['id'])
        for bkey in line['blocking_keys'].split(' '):
            ds1_ii[bkey].add(line['id'])
        ds1[line['id']] = convert_tokens(set(line['tokens'].split(' ')))

    for idx, line in enumerate(csv2):
        line['id'] = int(line['id'])
        ds2_size += 1
        ds2_ori_id_to_rec_id[extract_ori_rec_id(line['original_id'])].add(line['id'])
        for bkey in line['blocking_keys'].split(' '):
            ds2_ii[bkey].add(line['id'])
        ds2[line['id']] = convert_tokens(set(line['tokens'].split(' ')))

    print('----------------Summary----------------')
    true_pairs = []
    for k in (set(list(ds1_ori_id_to_rec_id.keys())) & set(list(ds2_ori_id_to_rec_id.keys()))):
        for i in ds1_ori_id_to_rec_id[k]:
            for j in ds2_ori_id_to_rec_id[k]:
                true_pairs.append((i,j))
    print('Total true pairs: {}'.format(len(true_pairs)))
    print('ds1 size: {}, ds2 size: {}'.format(ds1_size, ds2_size))
    print('Full pairwise comparisons: {}'.format(ds1_size * ds2_size))

    if args.blocking:
        print('------------Blocking enabled------------')
        print('Blocking key size (per record): {}'.format(bkey_per_record))
        print('Threshold: {}'.format(args.threshold))
        print('Unique blocking keys in ds1 / max possible: {} / {}'.format(len(ds1_ii), bkey_per_record * ds1_size))
        print('Unique blocking keys in ds2 / max possible: {} / {}'.format(len(ds2_ii), bkey_per_record * ds2_size))

        blocked_comps = set()
        shared_bkeys = set(list(ds1_ii.keys())) & set(list(ds2_ii.keys()))
        print('Shared blocking keys: {}'.format(len(shared_bkeys)))
        for k in shared_bkeys:
            for i in ds1_ii[k]:
                for j in ds2_ii[k]:
                    blocked_comps.add((i,j))
        blocked_comps = sorted(list(blocked_comps), key=lambda x: (x[0], x[1]))
        print('Blocked distinct comparisons: {}'.format(len(blocked_comps)))
        if args.verbose:
            print(blocked_comps)

        result = []
        for cand in blocked_comps:
            id1, id2 = cand
            score = jaccard(ds1[id1], ds2[id2])
            if score + EPSILON >= args.threshold:
                result.append(cand)
        print('Pairs found with blocking: {}'.format(len(result)))
        if args.verbose:
            print(result)

        not_in_block_true_pairs = list(set(true_pairs) - set(result))
        print('Not in block true pairs: {}'.format(len(not_in_block_true_pairs)))
        if args.verbose:
            print(not_in_block_true_pairs)

        pc, rr, f1 = compute_metrics(len(blocked_comps), ds1_size * ds2_size, 
                                    len(true_pairs) - len(not_in_block_true_pairs), len(true_pairs))
        print('PC: {}, RR: {}, F-score: {}'.format(pc, rr, f1))
