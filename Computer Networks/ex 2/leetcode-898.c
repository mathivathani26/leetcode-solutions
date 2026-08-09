class Solution {
public:
    int subarrayBitwiseORs(vector<int>& arr) {
        unordered_set<int> total_distinct; 
        unordered_set<int> current_ends;  
        for (int x : arr) {
            unordered_set<int> next_ends;
            next_ends.insert(x);
            for (int prev_or : current_ends) {
                next_ends.insert(prev_or | x);
            }
                for (int val : next_ends) {
                total_distinct.insert(val);
            }
            current_ends = move(next_ends);
        }
        return total_distinct.size();
    }
};
