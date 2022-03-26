#include "SimpleParallelNoClockAnalyzer.h"
#include "SimpleParallelNoClockAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

SimpleParallelAnalyzer::SimpleParallelAnalyzer()
:	Analyzer2(),  
	mSettings( new SimpleParallelAnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

SimpleParallelAnalyzer::~SimpleParallelAnalyzer()
{
	KillThread();
}

void SimpleParallelAnalyzer::SetupResults()
{
	mResults.reset( new SimpleParallelAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );

	//this is the channel where the BubbleText (the analyzer output values) will show up in the
	//waveform window
	mResults->AddChannelBubblesWillAppearOn( mSettings->mClockChannel );
}

void SimpleParallelAnalyzer::WorkerThread()
{
	//mSampleRateHz = GetSampleRate();

	mData.clear();
	mDataMasks.clear();
	mDataNextEdge.clear();

	//The sequence mDataChannels[0..15] from the settings corresponds always to D0..15.
	//The user may have selected 'None' for one or more Dx. This leads to mDataChannels[x]
	//being undefined.
	//The channel data is sorted in the sequence D0..D15 into mData, skipping undefined
	//channels during the process. The bit weight is kept in mDataMasks. (Skipping a Dx does
	//not alter the bit weights. Dx = 'None' means Dx = 0. Thus as if the bit is stuck at 0.)
	//In the following we have num_data_lines to process in a row.
	U32 count = (U32)mSettings->mDataChannels.size();
	for( U32 i=0; i<count; i++ )
	{
		if( mSettings->mDataChannels[i] != UNDEFINED_CHANNEL )
		{
			mData.push_back( GetAnalyzerChannelData( mSettings->mDataChannels[i] ) );
			mDataMasks.push_back( 1 << i );
			mDataChannels.push_back( mSettings->mDataChannels[i] );
		}
	}

	U32 num_data_lines = (U32)mData.size();

	//start at current position (leftmost, regardless of edge)
	U64 uiNearestEdge = mData[0]->GetSampleNumber();

	//mDataNextEdge stores edge positions from the previous loop cycle
	//to increase performance.
	for (U32 i = 0; i < num_data_lines; i++)
	{
		mDataNextEdge.push_back(uiNearestEdge);
	}

	for( ; ; )
	{
		//sample holds the current position in the waveform
		//uiNearestEdge is the next edge
		U64 sample = uiNearestEdge; //advance to next edge

		U16 result = 0;

		//advance all channels to the next edge, build-up their value
		//and add marker
		for( U32 i=0; i<num_data_lines; i++ )
		{
			mData[i]->AdvanceToAbsPosition( sample );
			if( mData[i]->GetBitState() == BIT_HIGH )
			{
				result |= mDataMasks[i];
			}
			mResults->AddMarker( sample, AnalyzerResults::Dot, mDataChannels[i] );
		}	

		//Find the next nearest edge of all channels.
		//If there are no more edges left uiNearestEdge stays at UINT64_MAX.
		uiNearestEdge = UINT64_MAX;
		for (U32 i = 0; i<num_data_lines; i++)
		{
			//mDataNextEdge needs to be updated only if we advanced with the
			//current position up to the stored edge position.
			//mDataNextEdge is UINT64_MAX if there are no more edges left.
			if (mDataNextEdge[i] <= sample) {
				//check beforehand if there is an edge at all, otherwise the thread will be stopped
				//if we approach the last sample with GetSampleOfNextEdge
				if (mData[i]->DoMoreTransitionsExistInCurrentData()) {
					mDataNextEdge[i] = mData[i]->GetSampleOfNextEdge();
				}
				else {
					mDataNextEdge[i] = UINT64_MAX; //flag no more edges left
				}
			}

			if (mDataNextEdge[i] < uiNearestEdge) {
				uiNearestEdge = mDataNextEdge[i];
			}
		}

		//add frame
		//note that with uiNearestEdge = UINT64_MAX the last value is added as TabularText
		//but not displayed as BubbleText
		Frame frame;
		frame.mData1 = result;
		frame.mFlags = 0;
		frame.mStartingSampleInclusive = sample;
		frame.mEndingSampleInclusive = uiNearestEdge;
		mResults->AddFrame( frame );

		mResults->CommitResults();

		ReportProgress( frame.mEndingSampleInclusive );
	}
}

bool SimpleParallelAnalyzer::NeedsRerun()
{
	return false;
}

U32 SimpleParallelAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 SimpleParallelAnalyzer::GetMinimumSampleRateHz()
{
	return 1000000;
}

const char* SimpleParallelAnalyzer::GetAnalyzerName() const
{
	return ::GetAnalyzerName();
}

const char* GetAnalyzerName()
{
	return "Simple Parallel noclock";
}

Analyzer* CreateAnalyzer()
{
	return new SimpleParallelAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}
