//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//
// Ice version 3.7.10
//
// <auto-generated>
//
// Generated from file `Communicator.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//

using _System = global::System;

#pragma warning disable 1591

namespace Ice
{
    [global::System.Runtime.InteropServices.ComVisible(false)]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1707")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1709")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1710")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1711")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1715")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1716")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1720")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1722")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1724")]
    public partial interface Communicator : global::System.IDisposable
    {
        #region Slice operations

        /// <summary>
        /// Destroy the communicator.
        /// This operation calls shutdown implicitly. Calling destroy cleans up
        ///  memory, and shuts down this communicator's client functionality and destroys all object adapters. Subsequent
        ///  calls to destroy are ignored.
        /// </summary>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        void destroy();

        /// <summary>
        /// Shuts down this communicator's server functionality, which includes the deactivation of all object adapters.
        /// Attempts to use a deactivated object adapter raise ObjectAdapterDeactivatedException. Subsequent calls to
        ///  shutdown are ignored.
        ///  After shutdown returns, no new requests are processed. However, requests that have been started before shutdown
        ///  was called might still be active. You can use waitForShutdown to wait for the completion of all
        ///  requests.
        /// </summary>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        void shutdown();

        /// <summary>
        /// Wait until the application has called shutdown (or destroy).
        /// On the server side, this
        ///  operation blocks the calling thread until all currently-executing operations have completed. On the client
        ///  side, the operation simply blocks until another thread has called shutdown or destroy.
        ///  A typical use of this operation is to call it from the main thread, which then waits until some other thread
        ///  calls shutdown. After shut-down is complete, the main thread returns and can do some cleanup work
        ///  before it finally calls destroy to shut down the client functionality, and then exits the application.
        /// </summary>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        void waitForShutdown();

        /// <summary>
        /// Check whether communicator has been shut down.
        /// </summary>
        /// <returns>True if the communicator has been shut down; false otherwise.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        bool isShutdown();

        /// <summary>
        /// Convert a stringified proxy into a proxy.
        /// For example, MyCategory/MyObject:tcp -h some_host -p 10000 creates a proxy that refers to the Ice
        ///  object having an identity with a name "MyObject" and a category "MyCategory", with the server running on host
        ///  "some_host", port 10000. If the stringified proxy does not parse correctly, the operation throws one of
        ///  ProxyParseException, EndpointParseException, or IdentityParseException. Refer to the Ice manual for a detailed
        ///   description of the syntax supported by stringified proxies.
        /// </summary>
        ///  <param name="str">The stringified proxy to convert into a proxy.
        ///  </param>
        /// <returns>The proxy, or nil if str is an empty string.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        ObjectPrx stringToProxy(string str);

        /// <summary>
        /// Convert a proxy into a string.
        /// </summary>
        /// <param name="obj">The proxy to convert into a stringified proxy.
        ///  </param>
        /// <returns>The stringified proxy, or an empty string if
        ///  obj is nil.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        string proxyToString(ObjectPrx obj);

        /// <summary>
        /// Convert a set of proxy properties into a proxy.
        /// The "base" name supplied in the property argument
        ///  refers to a property containing a stringified proxy, such as MyProxy=id:tcp -h localhost -p 10000.
        ///  Additional properties configure local settings for the proxy, such as MyProxy.PreferSecure=1. The
        ///  "Properties" appendix in the Ice manual describes each of the supported proxy properties.
        /// </summary>
        ///  <param name="property">The base property name.
        ///  </param>
        /// <returns>The proxy.</returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        ObjectPrx propertyToProxy(string property);

        /// <summary>
        /// Convert a proxy to a set of proxy properties.
        /// </summary>
        /// <param name="proxy">The proxy.
        ///  </param>
        /// <param name="property">The base property name.
        ///  </param>
        /// <returns>The property set.</returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        global::System.Collections.Generic.Dictionary<string, string> proxyToProperty(ObjectPrx proxy, string property);

        /// <summary>
        /// Convert an identity into a string.
        /// </summary>
        /// <param name="ident">The identity to convert into a string.
        ///  </param>
        /// <returns>The "stringified" identity.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        string identityToString(Identity ident);

        /// <summary>
        /// Create a new object adapter.
        /// The endpoints for the object adapter are taken from the property
        ///  name.Endpoints.
        ///  It is legal to create an object adapter with the empty string as its name. Such an object adapter is accessible
        ///  via bidirectional connections or by collocated invocations that originate from the same communicator as is used
        ///  by the adapter. Attempts to create a named object adapter for which no configuration can be found raise
        ///  InitializationException.
        /// </summary>
        ///  <param name="name">The object adapter name.
        ///  </param>
        /// <returns>The new object adapter.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        ObjectAdapter createObjectAdapter(string name);

        /// <summary>
        /// Create a new object adapter with endpoints.
        /// This operation sets the property
        ///  name.Endpoints, and then calls createObjectAdapter. It is provided as a
        ///  convenience function. Calling this operation with an empty name will result in a UUID being generated for the
        ///  name.
        /// </summary>
        ///  <param name="name">The object adapter name.
        ///  </param>
        /// <param name="endpoints">The endpoints for the object adapter.
        ///  </param>
        /// <returns>The new object adapter.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        ObjectAdapter createObjectAdapterWithEndpoints(string name, string endpoints);

        /// <summary>
        /// Create a new object adapter with a router.
        /// This operation creates a routed object adapter.
        ///  Calling this operation with an empty name will result in a UUID being generated for the name.
        /// </summary>
        ///  <param name="name">The object adapter name.
        ///  </param>
        /// <param name="rtr">The router.
        ///  </param>
        /// <returns>The new object adapter.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        ObjectAdapter createObjectAdapterWithRouter(string name, RouterPrx rtr);

        /// <summary>
        /// Get the implicit context associated with this communicator.
        /// </summary>
        /// <returns>The implicit context associated with this communicator; returns null when the property Ice.ImplicitContext
        ///  is not set or is set to None.</returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        ImplicitContext getImplicitContext();

        /// <summary>
        /// Get the properties for this communicator.
        /// </summary>
        /// <returns>This communicator's properties.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        Properties getProperties();

        /// <summary>
        /// Get the logger for this communicator.
        /// </summary>
        /// <returns>This communicator's logger.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        Logger getLogger();

        /// <summary>
        /// Get the observer resolver object for this communicator.
        /// </summary>
        /// <returns>This communicator's observer resolver object.</returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        global::Ice.Instrumentation.CommunicatorObserver getObserver();

        /// <summary>
        /// Get the default router for this communicator.
        /// </summary>
        /// <returns>The default router for this communicator.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        RouterPrx getDefaultRouter();

        /// <summary>
        /// Set a default router for this communicator.
        /// All newly created proxies will use this default router. To disable
        ///  the default router, null can be used. Note that this operation has no effect on existing proxies.
        ///  You can also set a router for an individual proxy by calling the operation ice_router on the
        ///  proxy.
        /// </summary>
        ///  <param name="rtr">The default router to use for this communicator.
        ///  </param>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        void setDefaultRouter(RouterPrx rtr);

        /// <summary>
        /// Get the default locator for this communicator.
        /// </summary>
        /// <returns>The default locator for this communicator.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        LocatorPrx getDefaultLocator();

        /// <summary>
        /// Set a default Ice locator for this communicator.
        /// All newly created proxy and object adapters will use this
        ///  default locator. To disable the default locator, null can be used. Note that this operation has no effect on
        ///  existing proxies or object adapters.
        ///  You can also set a locator for an individual proxy by calling the operation ice_locator on the
        ///  proxy, or for an object adapter by calling ObjectAdapter.setLocator on the object adapter.
        /// </summary>
        ///  <param name="loc">The default locator to use for this communicator.
        ///  </param>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        void setDefaultLocator(LocatorPrx loc);

        /// <summary>
        /// Get the plug-in manager for this communicator.
        /// </summary>
        /// <returns>This communicator's plug-in manager.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        PluginManager getPluginManager();

        /// <summary>
        /// Get the value factory manager for this communicator.
        /// </summary>
        /// <returns>This communicator's value factory manager.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        ValueFactoryManager getValueFactoryManager();

        /// <summary>
        /// Flush any pending batch requests for this communicator.
        /// This means all batch requests invoked on fixed proxies
        ///  for all connections associated with the communicator. Any errors that occur while flushing a connection are
        ///  ignored.
        /// </summary>
        ///  <param name="compress">Specifies whether or not the queued batch requests should be compressed before being sent over
        ///  the wire.</param>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        void flushBatchRequests(CompressBatch compress);

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        global::System.Threading.Tasks.Task flushBatchRequestsAsync(CompressBatch compress, global::System.IProgress<bool> progress = null, global::System.Threading.CancellationToken cancel = new global::System.Threading.CancellationToken());

        /// <summary>
        /// Add the Admin object with all its facets to the provided object adapter.
        /// If Ice.Admin.ServerId is
        ///  set and the provided object adapter has a Locator, createAdmin registers the Admin's Process facet with
        ///  the Locator's LocatorRegistry. createAdmin must only be called once; subsequent calls raise
        ///  InitializationException.
        /// </summary>
        ///  <param name="adminAdapter">The object adapter used to host the Admin object; if null and Ice.Admin.Endpoints is set,
        ///  create, activate and use the Ice.Admin object adapter.
        ///  </param>
        /// <param name="adminId">The identity of the Admin object.
        ///  </param>
        /// <returns>A proxy to the main ("") facet of the Admin object. Never returns a null proxy.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        ObjectPrx createAdmin(ObjectAdapter adminAdapter, Identity adminId);

        /// <summary>
        /// Get a proxy to the main facet of the Admin object.
        /// getAdmin also creates the Admin object and creates and
        ///  activates the Ice.Admin object adapter to host this Admin object if Ice.Admin.Enpoints is set. The identity of
        ///  the Admin object created by getAdmin is {value of Ice.Admin.InstanceName}/admin, or {UUID}/admin when
        ///  Ice.Admin.InstanceName is not set. If Ice.Admin.DelayCreation is 0 or not set, getAdmin is called
        ///  by the communicator initialization, after initialization of all plugins.
        /// </summary>
        ///  <returns>A proxy to the main ("") facet of the Admin object, or a null proxy if no Admin object is configured.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        ObjectPrx getAdmin();

        /// <summary>
        /// Add a new facet to the Admin object.
        /// Adding a servant with a facet that is already registered throws
        ///  AlreadyRegisteredException.
        /// </summary>
        ///  <param name="servant">The servant that implements the new Admin facet.
        ///  </param>
        /// <param name="facet">The name of the new Admin facet.</param>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        void addAdminFacet(Object servant, string facet);

        /// <summary>
        /// Remove the following facet to the Admin object.
        /// Removing a facet that was not previously registered throws
        ///  NotRegisteredException.
        /// </summary>
        ///  <param name="facet">The name of the Admin facet.
        ///  </param>
        /// <returns>The servant associated with this Admin facet.</returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        Object removeAdminFacet(string facet);

        /// <summary>
        /// Returns a facet of the Admin object.
        /// </summary>
        /// <param name="facet">The name of the Admin facet.
        ///  </param>
        /// <returns>The servant associated with this Admin facet, or null if no facet is registered with the given name.</returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        Object findAdminFacet(string facet);

        /// <summary>
        /// Returns a map of all facets of the Admin object.
        /// </summary>
        /// <returns>A collection containing all the facet names and servants of the Admin object.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        global::System.Collections.Generic.Dictionary<string, Object> findAllAdminFacets();

        #endregion
    }

    /// <summary>
    /// The output mode for xxxToString method such as identityToString and proxyToString.
    /// The actual encoding format for
    ///  the string is the same for all modes: you don't need to specify an encoding format or mode when reading such a
    ///  string.
    /// </summary>

    [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
    public enum ToStringMode
    {
        /// <summary>
        /// Characters with ordinal values greater than 127 are kept as-is in the resulting string.
        /// Non-printable ASCII
        ///  characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or \\unnnn.
        /// </summary>

        Unicode,
        /// <summary>
        /// Characters with ordinal values greater than 127 are encoded as universal character names in the resulting
        ///  string: \\unnnn for BMP characters and \\Unnnnnnnn for non-BMP characters.
        /// Non-printable ASCII characters
        ///  with ordinal values 127 and below are encoded as \\t, \\n (etc.) or \\unnnn.
        /// </summary>

        ASCII,
        /// <summary>
        /// Characters with ordinal values greater than 127 are encoded as a sequence of UTF-8 bytes using octal escapes.
        /// Characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or an octal escape. Use this mode
        ///  to generate strings compatible with Ice 3.6 and earlier.
        /// </summary>

        Compat
    }
}

namespace Ice
{
}
