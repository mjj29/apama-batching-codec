# apama-batching-codec

Apama connectivity codec to convert between a batch of events and a single message containing multiple events

## Supported Apama version

This works with Apama 10.3 or later minor version (referred to below as 10.3+)

## Prerequisites

Apama 10.3+ has been installed locally and APAMA_HOME, APAMA_WORK have been set in the environment.

A suitable C++ toolchain must be available.

## Building the plugin

Copy the content of the repository to a suitable folder and cd to it.

In an Apama command prompt on Linux run:

    mkdir -p $APAMA_WORK/lib
    g++ -std=c++11 -o $APAMA_WORK/lib/libconnectivity-batching-codec.so -I$APAMA_HOME/include -L$APAMA_HOME/lib -lapclient -I. -shared -fPIC BatchingCodec.cpp

On Windows run:

    mkdir %APAMA_WORK%\lib
    g++ -std=c++11 -o %APAMA_WORK%\lib\connectivity-batching-codec.dll -I%APAMA_HOME%\include -L%APAMA_HOME%\lib -lapclient -I. -shared BatchingCodec.cpp

## Building using Docker

There is a provided Dockerfile which will build the plugin, run tests and produce an image which is your base image, plus the CSV plugin. Application images can then be built from this image. To build the image run:

    docker build -t apama_with_batching_plugin .

By default the public docker images from Docker Store for 10.3 will be used. To use another version run:

    docker build -t apama_with_batching_plugin --build-arg APAMA_VERSION=10.5 .

To use custom images from your own repository then use:

    docker build -t apama_with_batching_plugin --build-arg APAMA_BUILDER=builderimage --build-arg APAMA_IMAGE=runtimeimage .

## Running tests

To run the tests for the plugin you will need to use an Apama command prompt to run the tests from within the tests directory:

    pysys run

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

The batching codec takes any payloads on the host side and converts to/from a list of those payloads on the transport side. This will need to be handled by something which accepts a list, such as the JSON codec. Any messages delivered as a single batch will be combined into a list. This is dependent on the load on the system at any given time. Lists may contain one item or many items. The exact number will vary between multiple runs. Each message containing a list produced by the transport will be expanded into a separate batch delivered to the host.

The batching codec has several options for handling metadata for the messages. This behaviour is controlled with the `metadataMode` configuration item to the codec:

    batchingChain:
        # ...
        batchingCodec:
            metadataMode: member
        # ...

The four options are:

1. **first**: This is the default mode. It assumes the metadata on all messages is identical (or irrelevant) and copies the metadata from the first message in the batch to the combined message, and the metadata from the combined message to every message in the batch it creates.
2. **member**: Instead of just putting the payload in the list, instead create a map `{ "metadata": messageMetadata, "payload": messagePayload }`
3. **requestIdList**: Copy the metadata as in **first**, but set the `metadata.requestId` field to be a list containing all the individual request id fields from each message, in the same order. Responses will be mapped in the reverse direction.
4. **splitBatch**: Check that the metadata is the same on each subsequent message and start a new batch whenever it changes.
