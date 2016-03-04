#include<stdio.h>
#include<string>
#include<stdlib.h>
#include<assert.h>
int *make_next(const char p[])
{
	int m = strlen(p), k = 0;
	int *next = (int *)calloc(m, sizeof(int));
	assert(next); 
	for(int i = 1; i < m; i++){
		while(k > 0 && p[k] != p[i])
			k = next[k - 1];
		if(p[k] == p[i])
			k++;
		next[i] = k;
	}
	return next;
}

int KMP_mathcer(const char ch1[], const char ch2[])
{
	int n = 0, m = 0, q = 0;
	if(0 == (n = strlen(ch1)) && 0 == (m = strlen(ch2)))
		return 0;
	int *next = make_next(ch2);
	
	for(int i = 0; i < n; i++){
		while(q > 0 && ch1[i] != ch2[q])
			q = next[q - 1];
		if(ch1[i] == ch2[q])
			q++;
		if(q == m){
			free(next);
			next = NULL;
			return i - m + 1;
		}
	}
	return 0;
}