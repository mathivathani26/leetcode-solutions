class Solution {
public:
    bool hasAlternatingBits(int n) {
        long all_ones = n ^ (n >> 1);
        return (all_ones & (all_ones + 1)) == 0;
    }
};
