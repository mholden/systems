#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <queue>
#include <iostream>

using namespace std;

// use a priority queue

typedef struct queue_entry {
    vector<int> schedule;
    int difficulty;
    int days_left;
} qe_t;

static void dump_qe(qe_t& qe) {
    cout << "days left " << qe.days_left << " difficulty " << qe.difficulty <<  " schedule: ";
    for (int i = 0; i < qe.schedule.size(); i++) {
        cout << qe.schedule[i] << " ";
    }
    cout << endl;
}

#if 0
static int day_difficulty(int sind, int eind, vector<int>& jd) {
    int max = 0;
    for (int i = sind + 1; i <= eind; i++) {
        if (jd[i] > max)
            max = jd[i];
    }
    return max;
}

static int schedule_difficulty(vector<int>& s, vector<int>& jd) {
    int sd = 0;
    sd = day_difficulty(-1, s[0], jd);
    for (int i = 1; i < s.size(); i++) {
        sd += day_difficulty(s[i-1], s[i], jd);
    }
    return sd;
}
#endif

static int **day_difficulty;

struct pq_cmp {
    bool operator()(qe_t &a, qe_t &b) {
        return a.difficulty > b.difficulty;
    }
};

int minDifficulty(vector<int>& jd, int d) {
    priority_queue<qe_t, vector<qe_t>, pq_cmp> pq;
    qe_t qe, new_qe;
    vector<int> schedule;
    int min = -1, days_left, last, max;
    
    if (jd.size() < d) // no solution
        goto out;
    
    // pre-compute all day difficulties
    assert(day_difficulty = (int **)malloc(jd.size() * sizeof(int *)));
    for (int i = 0; i < jd.size(); i++)
        assert(day_difficulty[i] = (int *)malloc(jd.size() * sizeof(int)));

    day_difficulty[0][0] = jd[0];
    for (int i = 0; i < jd.size(); i++) {
        max = 0;
        for (int j = i; j < jd.size(); j++) {
            if (jd[j] > max)
                max = jd[j];
            day_difficulty[i][j] = max;
        }
    }
    
    days_left = d;
    
    // add all first day options:
    if (days_left == 1) {
        min = day_difficulty[0][jd.size() - 1];
        goto out;
    }
    for (int i = 0; i < jd.size() - days_left + 1; i++) {
        schedule.clear();
        schedule.push_back(i);
        qe.schedule = schedule;
        qe.difficulty = day_difficulty[0][i];
        qe.days_left = days_left - 1;
        pq.push(qe);
    }
    
    // explore all schedules:
    while (!pq.empty()) {
        qe = pq.top(); pq.pop();
        if (min >= 0 && qe.difficulty >= min)
            break;
        //dump_qe(qe);
        if (qe.days_left == 1) {
            qe.schedule.push_back(jd.size() - 1);
            qe.difficulty += day_difficulty[qe.schedule[qe.schedule.size() - 2] + 1][qe.schedule[qe.schedule.size() - 1]];
            if (min < 0 || qe.difficulty < min)
                min = qe.difficulty;
        } else {
            days_left = qe.days_left;
            last = qe.schedule[qe.schedule.size() - 1];
            for (int i = last + 1; i < jd.size() - days_left + 1; i++) {
                new_qe.schedule = qe.schedule;
                new_qe.schedule.push_back(i);
                new_qe.days_left = days_left - 1;
                new_qe.difficulty = qe.difficulty + day_difficulty[last + 1][i];
                if (min < 0 || new_qe.difficulty < min) // only explore paths that have a chance
                    pq.push(new_qe);
            }
        }
    }
    
    assert(min > 0);
    
out:
    return min;
}

static void test_prog1_specific_case1(void) {
    vector<int> v;
    
    v = {6, 5, 4, 3, 2, 1};
    assert(minDifficulty(v, 2) == 7);
    
    v = {10, 6, 4, 1, 12, 2, 14};
    printf("md %d\n", minDifficulty(v, 3));
    
    v = {380,302,102,681,863,676,243,671,651,612,162,561,394,856,601,30,6,257,921,405,716,126,158,476,889,699,668,930,139,164,641,801,480,756,797,915,275,709,161,358,461,938,914,557,121,964,315};
    printf("md %d\n", minDifficulty(v, 10));
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
