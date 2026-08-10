#define MAX_NODES 5000000 
int trie[MAX_NODES][2];
int nodeCount = 1;
void resetTrie() {
    trie[0][0] = 0;
    trie[0][1] = 0;
    nodeCount = 1;
}
void insert(int num) {
    int curr = 0;
    for (int i = 30; i >= 0; i--) { 
        int bit = (num >> i) & 1;
        if (!trie[curr][bit]) {
            trie[nodeCount][0] = 0;
            trie[nodeCount][1] = 0;
            trie[curr][bit] = nodeCount++;
        }
        curr = trie[curr][bit];
    }
}
int getMaxXOR(int num) {
    int curr = 0;
    int maxXOR = 0;
    for (int i = 30; i >= 0; i--) {
        int bit = (num >> i) & 1;
        int toggledBit = 1 - bit;
        
        if (trie[curr][toggledBit]) {
            maxXOR |= (1U << i);
            curr = trie[curr][toggledBit];
        } else {
            curr = trie[curr][bit];
        }
    }
    return maxXOR;
}
int findMaximumXOR(int* nums, int numsSize) {
    if (numsSize < 2) return 0;
    resetTrie();
    for (int i = 0; i < numsSize; i++) {
        insert(nums[i]);
    }
    int maxResult = 0;
    for (int i = 0; i < numsSize; i++) {
        int currentMax = getMaxXOR(nums[i]);
        if (currentMax > maxResult) {
            maxResult = currentMax;
        }
    }
    return maxResult;
}
