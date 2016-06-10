/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <QResizeEvent>
#include "EngineWindow.h"
namespace AeonGames
{
    EngineWindow::EngineWindow ( QWindow *parent ) : QWindow ( parent ), mTimer()
    {
        /* On
        Windows we want the window to own its device context.
        This code works on Qt 5.6.0, but if it does not
        or a new non Qt application needs to set it,
        a window class style seems to be mutable by using
        SetClassLongPtr( ..., GCL_STYLE, ... );
        instead.
        https://msdn.microsoft.com/en-us/library/windows/desktop/ms633589%28v=vs.85%29.aspx
        http://www.gamedev.net/topic/319124-ogl-and-windows/#entry3055268

        This flag is Windows specific, so Linux or Mac implementations
        probably just ignore it, I have not verified this.
        */
        auto current_flags = flags();
        if ( ! ( current_flags & Qt::MSWindowsOwnDC ) )
        {
            setFlags ( current_flags | Qt::MSWindowsOwnDC );
        }
        if ( !mAeonEngine.RegisterRenderingWindow ( winId() ) )
        {
            throw std::runtime_error ( "Window registration failed." );
        }
        connect ( &mTimer, SIGNAL ( timeout() ), this, SLOT ( requestUpdate() ) );
    }
    EngineWindow::~EngineWindow()
    {
        mAeonEngine.UnregisterRenderingWindow ( winId() );
    }
    void EngineWindow::resizeEvent ( QResizeEvent * aResizeEvent )
    {
        mAeonEngine.Resize ( winId(), aResizeEvent->size().width(), aResizeEvent->size().height() );
    }

    void EngineWindow::exposeEvent ( QExposeEvent * aExposeEvent )
    {
        Q_UNUSED ( aExposeEvent );
        if ( isExposed() )
        {
            mTimer.start ( 0 );
            mStopWatch.start();
        }
        else
        {
            mTimer.stop();
            mStopWatch.invalidate();
        }
    }
    bool EngineWindow::event ( QEvent * aEvent )
    {
        switch ( aEvent->type() )
        {
        case QEvent::UpdateRequest:
            mAeonEngine.Step ( 0.0 );
            return true;
        default:
            return QWindow::event ( aEvent );
        }
    }
}
