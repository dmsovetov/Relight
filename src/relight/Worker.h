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

#ifndef __Relight_Worker_H__
#define __Relight_Worker_H__

#include "Relight.h"

namespace relight {

    //! Job data.
    struct JobData {
        Worker*             m_worker;   //!< Parent worker.
        Job*                m_job;      //!< Parent job.
        Relight*            m_relight;  //!< Relight instance.
        const Scene*        m_scene;    //!< Scene instance to be processed.
        const Mesh*         m_mesh;     //!< Mesh instance from a scene.
        bake::BakeIterator* m_iterator; //!< Bake iterator.
    };

    //! Relight job.
    class Job {
    public:

        //! Executes a job.
        virtual void    execute( JobData* data ) = 0;
    };

    //! A job that bakes all scene objects.
    class FullBakeJob : public Job {
    public:

                        //! Constructs a FullBakeJob instance.
                        FullBakeJob( Job* job, const Workers& workers );

        //! Executes a job.
        virtual void    execute( JobData* data );

    private:

        //! Array of available workers.
        Workers         m_workers;

        //! A job to push to workers.
        Job*            m_job;
    };

    //! Relight basic worker.
    class Worker : public Progress {
    public:

                        //! Constructs a Worker instance
                        Worker( void );
        virtual         ~Worker( void );

        //! Pushes a new job to this worker.
        virtual void    push( Job* job, JobData* data );

        //! Waits for completion of this worker.
        virtual void    wait( void );
    };

} // namespace relight

#endif /* defined(__Relight_Worker_H__) */
