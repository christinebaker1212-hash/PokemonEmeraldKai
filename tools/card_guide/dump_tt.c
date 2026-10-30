#include <stdio.h>
#include "tt/tt.h"
#include "tt/collection.h"
#include "tt/npc.h"
static void S(const char *s){putchar('"');for(;s&&*s;s++){if(*s=='"'||*s=='\\')putchar('\\');if((unsigned char)*s>=32)putchar(*s);}putchar('"');}
static void L(const uint16_t *p){int f=1;putchar('[');for(;p&&*p;p++){printf("%s%u",f?"":",",*p);f=0;}putchar(']');}
int main(void){unsigned i,f=1;
  printf("{\"cards\":[");
  for(i=1;i<=TT_CARD_MAX;i++){const struct TtCard*c=&gTtCards[i];if(!c->name)continue;printf("%s{\"id\":%u,\"name\":",f?"":",",i);f=0;S(c->name);printf(",\"stars\":%u,\"order\":%u}",c->stars,c->order);}
  printf("],\"pools\":[");for(i=0;i<TT_REGIONS;i++){printf("%s",i?",":"");L(gTtRegionPool[i]);}
  printf("],\"shelves\":[");for(i=0;i<TT_REGIONS;i++){printf("%s",i?",":"");L(gTtSellerStock[i]);}
  printf("],\"specials\":[");for(i=0;i<TT_SPECIAL_COUNT;i++){printf("%s",i?",":"");L(gTtSpecialCards[i]);}
  printf("]}\n");return 0;}
