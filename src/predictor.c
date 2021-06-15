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

// Custom Tournament Predictor variables: (add-on variables to existing tournament predictor)
uint32_t gsharePatternTableSize; // 2^m 2-bit counters, taken from global br history bits
uint32_t* gsharePatternTable; // table of 2-bit counters, stores 2-bit branch predictions

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

// clears the last bit for a given input bit of width 'bitWidth'
uint32_t clearLastBit(uint32_t inputBits, uint32_t bitWidth) {
  uint32_t output = inputBits;
  output &= ~(1 << (bitWidth - 1)); 

  return output;
}

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  switch(bpType){
    int i;
    int j;
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

    case TOURNAMENT:
      // printf("Start initializing\n");
      result = 0;
  
      metaPredictorTableSize = 1 << lhistoryBits;
      localBranchHistTableSize = 1 << lhistoryBits; 
      localPatternTableSize = 1 << lhistoryBits;    
      globalHistoryBits = ghistoryBits;
      globalPredictorTableSize = 1 << globalHistoryBits;
      globalBranchHistory = 0; // assume all not taken at the start
      predictedResult = 0;

      metaPredictorTable = (uint32_t*)malloc( metaPredictorTableSize * sizeof(uint32_t) ); // alloc mem for meta predictor table
      localBranchHistTable = (uint32_t*)malloc( localBranchHistTableSize * sizeof(uint32_t) ); // alloc mem for local branch hist table
      localPatternTable = (uint32_t*)malloc( localPatternTableSize * sizeof(uint32_t) ); // alloc mem for local pattern table 
      globalPredictorTable = (uint32_t*)malloc( globalPredictorTableSize * sizeof(uint32_t) ); // alloc mem for global predictor table 

      for (i = 0; i < localBranchHistTableSize; i++) {
        metaPredictorTable[i] =  1; 
        localBranchHistTable[i] = 0; // holds n-bit local branch history. initially assume all NOT-TAKEN
        localPatternTable[i] =  1; // holds values bet. 0 and 3
      }

      // initialize global br predictor 
      for(j = 0; j < globalPredictorTableSize; j++){
        globalPredictorTable[j] =  1;
      }
      // printf("finish tournament initialization\n");

      // printf("meta table size is: %d\n", metaPredictorTableSize); // 1024
      // printf("local br hist table size is: %d\n", localBranchHistTableSize); // 1024
      // printf("local pattern table size is: %d\n", localPatternTableSize); // 1024
      // printf("global br predictor table size is: %d\n", globalPredictorTableSize); // 2^9 == 512 
    case CUSTOM: 
      // since no args taken for lhistory, ghistory bits, and pcIndexBits:
      lhistoryBits = 10;
      pcIndexBits = lhistoryBits;
      ghistoryBits = 13;

      metaPredictorTableSize = 1 << lhistoryBits; // 2^n elements large
      localBranchHistTableSize = 1 << lhistoryBits; 
      localPatternTableSize = 1 << lhistoryBits;    
      gsharePatternTableSize = 1 << ghistoryBits; // 2^m elements large
      // need 4 allocations: 
      // Meta Table, local Br Hist Table, Local Br pattern table, Gshare patter/prediction table      
      metaPredictorTable = (uint32_t*)malloc( metaPredictorTableSize * sizeof(uint32_t) ); // alloc mem for meta predictor table
      localBranchHistTable = (uint32_t*)malloc( localBranchHistTableSize * sizeof(uint32_t) ); // alloc mem for local branch hist table
      localPatternTable = (uint32_t*)malloc( localPatternTableSize * sizeof(uint32_t) ); // alloc mem for local pattern table 
      gsharePatternTable = (uint32_t*)malloc( gsharePatternTableSize * sizeof(uint32_t) );

      for (i = 0; i < localBranchHistTableSize; i++) {
        metaPredictorTable[i] =  1; 
        localBranchHistTable[i] = 0; // holds n-bit local branch history. initially assume all NOT-TAKEN
        localPatternTable[i] =  1; // holds values bet. 0 and 3
      }

      // initialize gshare br predictor default to 01: WN 
      for(j = 0; j < gsharePatternTableSize; j++){
        gsharePatternTable[j] =  1;
      }
      // printf("gsharePatternTableSize is: %d", gsharePatternTableSize);

      result = 0;  
      globalBranchHistory = 0; // assume all not taken at the start
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
  uint32_t metaBits;
  uint32_t metaTableIndex;
  uint32_t which_predictor;
  uint32_t localPatternTableIndex;
  uint32_t gsharePatternTableIdx;
  uint32_t m_bit_mask;
  uint32_t globalBranchPredictorIndex;
  uint32_t gshare_bit_mask;
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
      } 
      else{
        return NOTTAKEN;
      }
      break;

    case TOURNAMENT:
      // printf("tournament making prediction");
      result = 0;
      // the meta predictor chooses bet. Predictor 1 & 2 
      // predictedResult = 

      // printf("tournament making prediction\n");
      metaBits = create_bitMask(0, lhistoryBits - 1); // extract lhistory num of bits from 'pc' starting from bit 0 to bit lhistoryBits - 1
      metaTableIndex = metaBits & pc; // get n bits from pc and access meta-predictor table
      which_predictor = metaPredictorTable[metaTableIndex]; // 2 bit counter figure out which predictor to use: 0,1-->local   1,2--> global

      if (which_predictor > 1) { // we choose Predictor 1 (local predictor)
        // access local predictor at index 'metaTableIndex'
        localPatternTableIndex = localBranchHistTable[metaTableIndex]; // get ready to access local pattern table (table of 2^n 2-bit counters)
        result = localPatternTable[localPatternTableIndex];
        // check if SN, WN, WT, ST --- and return accordingly
        if (result > 1) 
          return TAKEN;
        else 
          return NOTTAKEN;
      }

      else { // if not, we chose Pred 2 (global predictor)
        // access global branch predictor values with global history bits
        m_bit_mask = create_bitMask(0, ghistoryBits - 1); // create m-bit bitmask
        globalBranchPredictorIndex = m_bit_mask & globalBranchHistory; // extract m bits from 'global branch history'
        result = globalPredictorTable[globalBranchPredictorIndex]; 
        // check if SN, WN, WT, ST --- and return accordingly
        if (result > 1) 
          return TAKEN;
        else 
          return NOTTAKEN;
      }
      break;
    case CUSTOM:
      result = 0;
      // choose a predictor (P1 or P2):
      // printf("CUSTOM makes prediction");
      metaBits = create_bitMask(0, lhistoryBits - 1); // extract lhistory num of bits from 'pc' starting from bit 0 to bit lhistoryBits - 1
      metaTableIndex = metaBits & pc; // get n bits from pc and access meta-predictor table
      which_predictor = metaPredictorTable[metaTableIndex]; // 2 bit counter figure out which predictor to use: 0,1-->local   1,2--> global

      if (which_predictor > 1) { // choose local predictor
        localPatternTableIndex = localBranchHistTable[metaTableIndex]; // get ready to access local pattern table (table of 2^n 2-bit counters)
      
        result = localPatternTable[localPatternTableIndex];
        if (result > 1) {
          return TAKEN;
        }
        else 
          return NOTTAKEN;
      }
      else { // else, choose gshare predictor
        gsharePatternTableIdx = globalBranchHistory ^ (pc & (gsharePatternTableSize - 1));
        // printf("gsharePatternTableIdx is: %d\n", gsharePatternTableIdx);
        if (gsharePatternTable[gsharePatternTableIdx] > 1) {
          return TAKEN;
        }
        else 
          return NOTTAKEN;
      }
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
  uint32_t n_bit_mask;
  uint32_t m_bit_mask;
  uint32_t localIdx;
  uint32_t localPatternIndex; // access local pattern
  uint32_t p1Prediction;
  uint32_t p2Prediction;
  uint32_t globalBranchPredictorIndex;
  uint32_t gsharePatternTableIdx;

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

      // UPDATE META TABLE VALUES
      // get P1 (local) prediction: 
      // printf("tournament training predictor\n");
      n_bit_mask = create_bitMask(0, lhistoryBits-1);// create n-bit mask
      localIdx = n_bit_mask & pc;
      // printf("local history bits %d\n", lhistoryBits); // shld be 10
      // printf("pc is %d\n", pc); 
      // printf("n_bit_mask is %d\n", n_bit_mask); // shld be 1023 in decimal
      // printf("localIdx is: %d\n", localIdx); // should be between 0 and 1023 (since 9:10:10)

      localPatternIndex = localBranchHistTable[localIdx];
      p1Prediction = NOTTAKEN;
      if (localPatternTable[localPatternIndex] > 1) {
        p1Prediction = TAKEN;
      }

      // get P2 (global) prediction: 
      m_bit_mask = create_bitMask(0, ghistoryBits - 1); // create m-bit bitmask
      globalBranchPredictorIndex = m_bit_mask & globalBranchHistory; // extract m bits from 'global branch history'
      // printf("global branch history before: %d\n", globalBranchHistory); 
      // printf("global history bits %d\n", ghistoryBits); // shld be 9
      // printf("m_bit_mask is %d\n", m_bit_mask);  // shld be 511 in decimal
      // printf("globalBr predictor index is: %d\n", globalBranchPredictorIndex); // should be between 0 and 511 (since 9:10:10)

      p2Prediction = NOTTAKEN;
      if (globalPredictorTable[globalBranchPredictorIndex] > 1) {
        p2Prediction = TAKEN;
      }

      // First: update META TABLE by checking p1 and p2 prediction values vs actual outcome
      if (p1Prediction == outcome && p2Prediction != outcome) { // if p1 correct, p2 wrong, we increment meta table to favor p1 (10, 11)
        if (metaPredictorTable[localIdx] < 3) {
          metaPredictorTable[localIdx]++;
        }
      }
      else if (p1Prediction != outcome && p2Prediction == outcome) { // if p1 wrong, p2 correct, we decrement meta table to favor p2 (00, 01)
        if (metaPredictorTable[localIdx] > 0) {
          metaPredictorTable[localIdx]--;
        }
      }
      // else P1 P2 both wrong/correct, dont change metaTable value
      // left shift by 1 bit (0) first

      // next: update local br predictor tables: i.e. local branch history table & local pattern table
      localBranchHistTable[localIdx] = clearLastBit(localBranchHistTable[localIdx], lhistoryBits); // clear n-th bit before inserting new one in
      localBranchHistTable[localIdx] <<= 1;
      if (outcome == TAKEN) {
        localBranchHistTable[localIdx] |= 1;
        if (localPatternTable[localPatternIndex] < 3) {
          localPatternTable[localPatternIndex]++;
        }
      }
      else { // outcome NOTTAKEN
         if (localPatternTable[localPatternIndex] > 0) {
          localPatternTable[localPatternIndex]--;
        } 
      }

      // next: update global br predictor values: i.e global branch history and global branch prediction table
      // printf("global branch history before %d\n", globalBranchHistory); 
      globalBranchHistory = clearLastBit(globalBranchHistory, globalHistoryBits); // clear m-th bit before inserting new one in
      globalBranchHistory <<= 1;  // global branch history has fixed number of bits       

      if (outcome == TAKEN) {
        globalBranchHistory |= 1;
        if (globalPredictorTable[globalBranchPredictorIndex] < 3) {
          globalPredictorTable[globalBranchPredictorIndex]++;
        }
      }
      else { // outcome NOTTAKEN
        if (globalPredictorTable[globalBranchPredictorIndex] > 0) {
          globalPredictorTable[globalBranchPredictorIndex]--;
        }
      }

      // printf("outcome is %d\n", outcome); 
      // printf("global branch history after %d\n", globalBranchHistory); 
      // printf("finished training!\n");
      break;

    case CUSTOM: 
      // printf("local history bits %d\n", lhistoryBits); // shld be 10
      // printf("pc is %d\n", pc); 
      // printf("n_bit_mask is %d\n", n_bit_mask); // shld be 1023 in decimal
      // printf("localIdx is: %d\n", localIdx); // should be between 0 and 1023 (since 9:10:10)

      // get P1 (local) prediction 
      n_bit_mask = create_bitMask(0, lhistoryBits-1);// create n-bit mask
      localIdx = n_bit_mask & pc;
      localPatternIndex = localBranchHistTable[localIdx];
      p1Prediction = NOTTAKEN;
      if (localPatternTable[localPatternIndex] > 1) {
        p1Prediction = TAKEN;
      }

      // get P2 (gshare) prediction 
      p2Prediction = NOTTAKEN;
      gsharePatternTableIdx = globalBranchHistory ^ (pc & (gsharePatternTableSize - 1));
      // printf("*gsharePatternTableIdx is: %d\n",gsharePatternTableIdx);
      if (gsharePatternTable[gsharePatternTableIdx] > 1) {
        p2Prediction = TAKEN;
      }

      // First: update META TABLE by checking p1 and p2 prediction values vs actual outcome
      if (p1Prediction == outcome && p2Prediction != outcome) { // if p1 correct, p2 wrong, we increment meta table to favor p1 (10, 11)
        if (metaPredictorTable[localIdx] < 3) {
          metaPredictorTable[localIdx]++;
        }
      }
      else if (p1Prediction != outcome && p2Prediction == outcome) { // if p1 wrong, p2 correct, we decrement meta table to favor p2 (00, 01)
        if (metaPredictorTable[localIdx] > 0) {
          metaPredictorTable[localIdx]--;
        }
      }

      // next: update local br predictor tables: i.e. local branch history table & local pattern table
      localBranchHistTable[localIdx] = clearLastBit(localBranchHistTable[localIdx], lhistoryBits); // clear n-th bit before inserting new one in
      localBranchHistTable[localIdx] <<= 1;
      if (outcome == TAKEN) {
        localBranchHistTable[localIdx] |= 1;
        if (localPatternTable[localPatternIndex] < 3) {
          localPatternTable[localPatternIndex]++;
        }
      }
      else { // outcome NOTTAKEN
         if (localPatternTable[localPatternIndex] > 0) {
          localPatternTable[localPatternIndex]--;
        } 
      }

      // next: update gshare pattern/predictor table and global branch history
      // globalBranchHistory = clearLastBit(globalBranchHistory, globalHistoryBits); // clear m-th bit before inserting new one in
      globalBranchHistory <<= 1;  // global branch history has fixed number of bits       

      if (outcome == TAKEN) {          
        globalBranchHistory |= 1; 
        if (gsharePatternTable[gsharePatternTableIdx] < 3) {
          gsharePatternTable[gsharePatternTableIdx]++;
        }
      }
      else { // outcome NOTTAKEN, decrement gshare 2-bit counter
        if (gsharePatternTable[gsharePatternTableIdx] > 0) {
          gsharePatternTable[gsharePatternTableIdx]--;
        }
      }
      
      globalBranchHistory = globalBranchHistory & (gsharePatternTableSize - 1);
      break;
    default: 
      break;
  }
}
