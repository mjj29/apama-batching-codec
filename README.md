# apama-batching-codec
Apama connectivity codec to convert between a batch of events and a single message containing multiple events

## Supported Apama version

This works with Apama 10.3 or later

## Building the plugin

In an Apama command prompt on Linux run:

    mkdir -p $APAMA_WORK/lib
    g++ -std=c++11 -o $APAMA_WORK/lib/libconnectivity-batching-codec.so -I$APAMA_HOME/include -L$APAMA_HOME/lib -lapclient -I. -shared -fPIC BatchingCodec.cpp

On Windows run:

    g++ -std=c++11 -o %APAMA_WORK%\lib\connectivity-batching-codec.dll -I%APAMA_HOME%\include -L%APAMA_HOME%\lib -lapclient -I. -shared BatchingCodec.cpp

## Building using Docker

There is a provided Dockerfile which will build the plugin, run tests and produce an image which is your base image, plus the CSV plugin. Application images can then be built from this image. To build the image run:

    docker build -t apama_with_batching_plugin .

By default the public docker images from Docker Store for 10.3 will be used (once 10.3 has been released). To use another version run:

    docker build -t apama_with_batching_plugin --build-arg APAMA_VERSION=10.1 .

To use custom images from your own repository then use:

    docker build -t apama_with_batching_plugin --build-arg APAMA_BUILDER=builderimage --build-arg APAMA_IMAGE=runtimeimage .

## Using the batching codec

As a codec in a connectivity chain you will need to first import the plugin into your configuration:

    connectivityPlugins:
	   batchingCodec:
        libraryName: connectivity-batching-codec
		  class: BatcherCodec

You can now use the batching codec in a chain definition:

    batchingChain:
	    - apama.eventMap
       - mapperCodec:
		    # ... mapping configuration
		 - batchingCodec
		 - jsonCodec
		 - stringCodec
		 - someTransport

The batching codec takes any payloads on the host side and converts to/from a list of those payloads on the trasport side. This will need to be handled by something which accepts a list, such as the JSON codec. Any messages delivered as a single batch will be combined into a list. This is dependent on the load on the system at any given time. Lists may contain one item or many items. The exact number will vary between multiple runs. Each message containing a list produced by the transport will be expanded into a separate batch delivered to the host.

The batching codec takes no options. Metadata for a batch of events will be copied from the first event to the combined event. Metadata from a combined event will be copied to each message in the resulting batch.

