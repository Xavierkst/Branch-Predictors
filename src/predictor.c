//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName1 = "Kai Tan";
const char *studentID1   = "A15719031";
const char *email1       = "ktan@ucsd.edu";

const char *studentName2 = "Yanbing Fang";
const char *studentID2   = "A59003982";
const char *email2       = "yafang@ucsd.edu";
//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

uint32_t preTabLength; 
uint32_t *preTab; // predictor table
uint32_t gHis; 
uint32_t result;

// Tournament predictor variables: 
// we need 1 Global Br Predictor, and 1 local branch predictor
uint32_t metaPredictorTableSize;
uint32_t *metaPredictorTable; // a table of 2-bit counters to choose one of the 2 predictors

uint32_t globalHistoryBits; 
uint32_t globalBranchHistory;
uint32_t globalPredictorTableSize; 
uint32_t *globalPredictorTable; // a table of 2 bit counter to choose between SNot-taken, WN, WT, STaken

// local branch predictor
uint32_t localBranchHistTableSize; 
uint32_t *localBranchHistTable; // a table of local branch histories
uint32_t localPatternTableSize; 
uint32_t *localPatternTable; // a table of local patterns indexed elements in the localPredictorTable

uint32_t predictedResult; // holds the predictor's prediction 

//------------------------------------//
//           Helper Functions         //
//------------------------------------//

// helper function to get bit mask from bits a thru b 
uint32_t create_bitMask(uint32_t a, uint32_t b) {
  uint32_t mask = 0;
  for (int j = a; j <= b; j++) {
    mask |= 1 << j;
  }

  return mask;
}

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  switch(bpType){
    int i = 0;

    case GSHARE:
      // printf("Start initializing\n");
      
      // printf("ghistoryBitsd:%d\n",ghistoryBits);
      preTabLength = 1 << ghistoryBits;

      preTab = (uint32_t*)malloc( preTabLength*sizeof(uint32_t) );    // predictor table
      for(i = 0; i < preTabLength; i++){    //initialized to WN
        preTab[i] = 1;
      }  

      gHis = 0;  //initialized to NOTTAKEN
      result = 0;
      break;
    case TOURNAMENT: 
      // meta and local table have the same # of bits, since both are use PC address to index into
      // initialize local br predictor 
      metaPredictorTableSize = 1 << lhistoryBits;
      localBranchHistTableSize = 1 << lhistoryBits; 
      localPatternTableSize = 1 << lhistoryBits;

      // allocate mem for all 3 tables
      metaPredictorTable = (uint32_t*)malloc( metaPredictorTableSize * sizeof(uint32_t) ); // alloc mem for meta predictor table
      localBranchHistTable = (uint32_t*)malloc( localBranchHistTableSize * sizeof(uint32_t) ); // alloc mem for meta predictor table
      localPatternTable = (uint32_t*)malloc( localPatternTableSize * sizeof(uint32_t) ); // alloc mem for local pattern table 

      // default values for each table is weakly not-taken (?)
      for (int i = 0; i < localBranchHistTableSize; i++) {
        // metaPredictorTable holds values bet. 0 and 3, 
        // each chooses between Predictor 1 and predictor 2. (eg. strongly P1, weakly P1, weakly P2, strongly P2..)
        metaPredictorTable[i] =  WN; 
        localBranchHistTable[i] = 0; // holds n-bit local branch history. initially assume all NOT-TAKEN
        localPatternTable[i] =  WN;// holds values bet. 0 and 3
      }

      // initialize global br predictor 
      globalHistoryBits = ghistoryBits;
      globalPredictorTableSize = 1 << globalHistoryBits;
      for (int i = 0; i < globalPredictorTableSize; i++) {
        globalPredictorTable[i] =  WN;// holds values bet. 0 and 3, indexed by globalHistory bits
      }

      globalBranchHistory = 0; 
      predictedResult = 0;
      break;
    default: 
      break;
  }


}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      // printf("GSHARE predicting \n");
      // printf("pc: %d\n",pc);

     result = gHis ^ (pc & (preTabLength - 1));
      // printf("result: %d, preTabLength%d\n",result,preTabLength);
      if(preTab[result] > 1){
        return TAKEN;
      }else{
        return NOTTAKEN;
      }
      break;
    case TOURNAMENT:
      // the meta predictor chooses bet. Predictor 1 & 2 
      // predictedResult = 

      uint32_t metaBits = create_bitMask(0, lhistoryBits - 1); // extract lhistory num of bits from 'pc' starting from bit 0 to bit lhistoryBits - 1
      uint32_t metaTableIndex = metaBits & pc; // get n bits from pc and access meta-predictor table
      uint32_t which_predictor = metaPredictorTable[metaTableIndex]; // 2 bit counter figure out which predictor to use: 0,1-->local   1,2--> global

      if (which_predictor > 1) { // we choose Predictor 1 (local predictor)
        // access local predictor at index 'which_predictor'
        uint32_t localPatternTableIndex = localBranchHistTable[which_predictor]; // get ready to access local pattern table (table of 2^n 2-bit counters)
        uint32_t result = localPatternTable[localPatternTableIndex];
        // check if SN, WN, WT, ST --- and return accordingly
        if (result > 1) 
          return TAKEN;
        else 
          return NOTTAKEN;
      }

      else { // if not, we chose Pred 2 (global predictor)
        // access global branch predictor values with global history bits
        uint32_t m_bit_mask = create_bitMask(0, ghistoryBits - 1); // create m-bit bitmask
        uint32_t globalBranchPredictorIndex = m_bit_mask & globalBranchHistory; // extract m bits from 'global branch history'
        uint32_t result = globalPredictorTable[globalBranchPredictorIndex]; 
        // check if SN, WN, WT, ST --- and return accordingly
        if (result > 1) 
          return TAKEN;
        else 
          return NOTTAKEN;
      }
      break;
    case CUSTOM:
      break;
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //

  switch(bpType) {
    case GSHARE:
      // printf("GSHARE training\n");
      // printf("pc: %d\n",pc);
      
      gHis <<=  1;
      if(outcome == TAKEN){
        gHis |= 1;
        if(preTab[result] < 3){
          preTab[result]++;
        }
      }else{
        if(preTab[result]>0){
          preTab[result]--;
        }
      }

      gHis = gHis & (preTabLength - 1);
      break;
    case TOURNAMENT: 
      
      break;
  }

}
