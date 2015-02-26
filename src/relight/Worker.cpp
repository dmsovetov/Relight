/**************************************************************************

 The MIT License (MIT)

 Copyright (c) 2015 Dmitry Sovetov

 https://github.com/dmsovetov

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

 **************************************************************************/

#include "BuildCheck.h"
#include "Worker.h"
#include "Scene.h"
#include "baker/Baker.h"

namespace relight {

// ** FullBakeJob::FullBakeJob
FullBakeJob::FullBakeJob( Job* job, const Workers& workers ) : m_workers( workers ), m_job( job )
{

}

// ** FullBakeJob::execute
void FullBakeJob::execute( JobData* data )
{
    int numWorkers = ( int )m_workers.size();

    for( int i = 0; i < data->m_scene->meshCount(); i++ ) {
        for( int j = 0, n = numWorkers; j < n; j++ ) {
            JobData* instanceData       = new JobData;
            instanceData->m_job         = m_job;
            instanceData->m_scene       = data->m_scene;
            instanceData->m_relight     = data->m_relight;
            instanceData->m_mesh        = data->m_scene->mesh( i );
            instanceData->m_iterator    = new bake::FaceBakeIterator( j, numWorkers );

            m_workers[i]->push( m_job, instanceData );
        }
    }

    for( int i = 0; i < numWorkers; i++ ) {
        m_workers[i]->wait();
    }
}

// ** Worker::Worker
Worker::Worker( Progress* progress ) : m_progress( progress )
{
}

Worker::~Worker( void )
{

}

// ** Worker::progress
Progress* Worker::progress( void ) const
{
    return m_progress;
}

// ** Worker::push
void Worker::push( Job* job, JobData* data )
{
    data->m_progress = m_progress;
    job->execute( data );
}

// ** Worker::wait
void Worker::wait( void )
{

}

} // namespace relight