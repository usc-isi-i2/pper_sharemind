import stdlib;
import shared3p;
import profiling;
import shared3p_sort;
import shared3p_join;

domain pd_shared3p shared3p;

float EPSILON = 0.000000001;


template <domain D>
D bool jaccard(D uint64 inter, uint64 union, D float t) {
    // jaccard(x, y) = len(x & y) / len(x | y) 
    //               = len(x & y) / (len(x) + len(y) - len(x & y))
    D float32 jaccard_result = (float32)inter / ((float32)union - (float32)inter);
    return (jaccard_result + EPSILON) >= t;
}

template <domain D>
D bool jaccard_naive(D uint64 [[1]] a, D uint64 [[1]] b, D float32 t) {
    D bool result = true;
    D uint64 match_counter = 0;
    D float32 jaccard_result = 0;
    for (uint64 i = 0; i < size(a); i++) {
        for (uint64 j = 0; j < size(b); j++) {
            match_counter += (uint64) (a[i] == b[j]);
        }
    }
    return jaccard(match_counter, size(a) + size(b), t);
}

template <domain D>
D bool jaccard_repeat(D uint64 [[1]] a, D uint64 [[1]] b, D float32 t) {
    // pre-processing
    // a = [1, 2, 3], b = [2, 4], extend both to be size(a) * size(b)
    // a_ext = [1, 2, 3, 1, 2, 3]
    // b_ext = [2, 2, 2, 4, 4, 4]
    // intersection = sum(a_ext == b_ext)
    uint64 [[1]] indices(size(a) * size(b));
    // a -> a_ext
    for (uint j = 0; j < size(b); j++) {
        indices[j*size(a) : (j+1)*size(a)] = iota(size(a));
    }
    D uint64 [[1]] a_ext(size(a) * size(b));
    __syscall("shared3p::gather_uint64_vec", __domainid(D), a, a_ext, __cref indices);
    // b -> a_ext
    for (uint j = 0; j < size(b); j++) {
        indices[j*size(a) : (j+1)*size(a)] = j;
    }
    D uint64 [[1]] b_ext(size(a) * size(b));
    __syscall("shared3p::gather_uint64_vec", __domainid(D), b, b_ext, __cref indices);

    D uint64 match_counter = sum(a_ext == b_ext);
    return jaccard(match_counter, size(a) + size(b), t);
}

template <domain D>
D bool jaccard_sort(D uint64 [[1]] a, D uint64 [[1]] b, D float32 t) {
    // both a and b can not have duplicated entries
    // a = [1, 2, 3], b = [3, 4]
    // c = [1, 2, 3, 3, 4]
    // c1 = [1, 2, 3, 3]
    // c2 = [2, 3, 3, 4]
    // intersection = sum(c1 == c2)
    D uint64 [[1]] c = cat(a, b);
    c = quicksort(c);
    D uint64 match_counter = sum(c[:size(c)-1] == c[1:]);
    return jaccard(match_counter, size(a) + size(b), t);
}

template <domain D>
D bool jaccard_join(D uint64 [[1]] a, D uint64 [[1]] b, D float32 t) {
    // Though the input type is uint64, it will be treated as uint32
    D xor_uint32 [[1]] a_xor32 = reshare((uint32)a);
    D xor_uint32 [[1]] b_xor32 = reshare((uint32)b);
    D xor_uint32 [[2]] a_ext (size(a_xor32), 1);
    D xor_uint32 [[2]] b_ext (size(b_xor32), 1);
    a_ext[:,0] = a_xor32;
    b_ext[:,0] = b_xor32;
    D xor_uint32 [[2]] c = tableJoinAes128(a_ext, (uint)0, b_ext, (uint)0);
    D uint64 match_counter = shape(c)[0];
    return jaccard(match_counter, size(a_xor32) + size(b_xor32), t);
}

void main() {
    pd_shared3p uint64 [[1]] a = argument("a");
    pd_shared3p uint64 [[1]] b = argument("b");
    pd_shared3p float32 t = argument("t");

    // pd_shared3p bool result = jaccard_naive(a, b, t);
    // pd_shared3p bool result = jaccard_repeat(a, b, t);
    pd_shared3p bool result = jaccard_sort(a, b, t);
    // pd_shared3p bool result = jaccard_join(a, b, t);

    publish("result", result);
}
