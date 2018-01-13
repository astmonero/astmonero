	// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
     boost::assign::map_list_of
	        (     0, uint256("0x909884bf9643e40fb25906c979a4bedb1f033ffb49d9d102901f6de3dbacfe68"))
		(   100, uint256("0x3955072e57a568e296337499f3897b2cb4bd87a2a1d420777ab44eb02127fae8"))
		(   200, uint256("0xfb22cce7a2f9a52ef6eb1feb294f634dca16ec278333959ba677084f82d44565"))
		(   300, uint256("0xfa9bed5a2415ebb1e4428e68dbcd324b0db1593a7848afa4649ac85435c97160"))
		(   400, uint256("0x50b7d0beb8d13c55a28bcc23ed9fb8bd54d2392f19f2ae0feaa0b6d9e69422fb"))
		(   500, uint256("0xf854036dcfb48d471c402e6f94af48506e849d42c513549b7d372e8d24cd969c"))
		(   600, uint256("0xa2f511679b5895bca89fc74a8f694445d57adf13847fd140653e240ecddeffc0"))
		(   700, uint256("0xc46d3040715d6f815fafd72a7f8467a3e3a6f77775f0f8efc9133c8ceeea2afd"))
		(   800, uint256("0xa4a1398387be6b0b7898db08fc372d98bde09fff4204ccf19d787d83d7b23689"))
		(   900, uint256("0x967b63b89476c04e35d83fa3c735ba9c8c6d441e8bf35437c0a3c66cb21249d5"))
		(  1000, uint256("0x888f6b7a29e0d4863be66f12b35b90920560034d81448024f4e537e3cff99c14"))
		(  1200, uint256("0x7de2a7e2235d9c41c7ea4702024872796668e9d656586ac99a3eaba0eae97dc0"))
		(  1400, uint256("0xc3e9bf41e4a66d51c256a5bf08b1cbd67ca2d2107f0a76d3fab15126167ef356"))
		(  1546, uint256("0x8c7aed8732193fc7e35614a439d69fa38590ca1449abf1e7800f8e4628924705"))
;
		

    static const CCheckpointData data = {
        &mapCheckpoints,
        1515765867, // * UNIX timestamp of last checkpoint block
        1547,    // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        1000.0     // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        (   0, uint256("0x"))
        
        ;
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        //1492691811,
        //547,
        //576
    };

    const CCheckpointData &Checkpoints() {
        if (fTestNet)
            return dataTestnet;
        else
            return data;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}
