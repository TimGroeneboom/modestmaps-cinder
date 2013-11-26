/*
 *  TileLoader.cpp
 *  modestmaps-ci
 *
 *  Created by Tom Carden on 8/27/10.
 *  Copyright 2010 Stamen Design. All rights reserved.
 *
 */

#include "TileLoader.h"
#include "cinder/app/app.h"
#include "cinder/ImageIo.h"

#include "cinder/Utilities.h"

#if defined( CINDER_COCOA )
#include <objc/objc-auto.h>
#endif

namespace cinder { namespace modestmaps {

void TileLoader::doThreadedPaint( const Coordinate &coord )
{
	
	ci::app::console() << "loading " << coord << std::endl;
	Surface image;
    
    if (provider) {
        image = provider->createSurface( coord );
    }
    
	pendingCompleteMutex.lock();
    if (pending.count(coord) > 0) {
        if (image) {
            completed[coord] = image;
        }
        pending.erase(coord);  
    } // otherwise clear was called so we should abandon this image to the ether
	pendingCompleteMutex.unlock();
}

void TileLoader::processQueue(std::vector<Coordinate> &queue )
{
	//ci::app::console() << "processQueue size " << queue.size() << std::endl;
	while (queue.size() > 0) {
		Coordinate key = *(queue.begin());
		queue.erase(queue.begin());
	//	ci::app::console() << "processQueue size " << queue.size() << std::endl;
       // ci::app::console() << "loading " << key << std::endl;
		Surface image;
    
		if (provider) {
			image = provider->createSurface( key );
		}
		if (image) {
			completed[key] = image;
			//	 ci::app::console() << "loaded image for " << key << std::endl;
		}
	}
}

void TileLoader::transferTextures(std::map<Coordinate, gl::Texture> &images)
{
    // use try_lock because we can just wait until next frame if needed
   
    while (!completed.empty()) {
        std::map<Coordinate, Surface>::iterator iter = completed.begin();
        if (iter->second) {
            images[iter->first] = gl::Texture(iter->second);		
        }
        completed.erase(iter);
    }
       
}
    
bool TileLoader::isPending(const Coordinate &coord)
{
    bool coordIsPending = false;
    pendingCompleteMutex.lock();
    coordIsPending = (pending.count(coord) > 0);
    pendingCompleteMutex.unlock();
    return coordIsPending;
}
    
void TileLoader::setMapProvider( MapProviderRef _provider )
{
	pendingCompleteMutex.lock();
    completed.clear();
    pending.clear();
	pendingCompleteMutex.unlock();
    provider = _provider;
}

} } // namespace