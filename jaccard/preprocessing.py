import sys


def ngram(n, s, place_holder=' ', padded=False):
    if len(s) == 0:
        return []
    if padded:
        pad = place_holder * (n - 1)
        s = pad + s + pad
    s = s.split(' ')
    s = place_holder.join(s)
    if len(s) < n:
        return [s]
    return [s[i:i + n] for i in range(len(s) - n + 1)]


def encode_token(t):
    """
    example: ab -> 0x6162
    """

    # uint64 maxsize = 8 bytes
    if len(t) >= 8:
        raise ValueError('Max length of character is 8')

    re = 0
    for idx, c in enumerate(reversed(t)):
        re += (ord(c) & 0xff) << (8 * idx)
    return re


if __name__ == '__main__':

    a = sys.argv[1]
    b = sys.argv[2]
    n = int(sys.argv[3]) # n-gram

    a = set(ngram(n, a.lower()))
    b = set(ngram(n, b.lower()))

    jaccard_score = len(a & b) / len(a | b)
    print('a:', a)
    print('b:', b)
    print('jaccard similarity score:', jaccard_score)

    enc_a = [encode_token(t) for t in a]
    print('encoded a: [{}]'.format(', '.join([hex(e) for e in enc_a])))
    enc_b = [encode_token(t) for t in b]
    print('encoded b: [{}]'.format(', '.join([hex(e) for e in enc_b])))


