﻿using System;
using System.Collections.Concurrent;
using Warpr.Configuration;
using Warpr.Gateway.Messages;
using Warpr.Gateway.Messaging;
using Warpr.Gateway.Sources;

namespace Warpr.Gateway.Session
{
  public class SourceSinkPair : IDisposable
  {
    public WebSocketConnection Source { get; }
    public WebSocketConnection Sink { get; }

    bool _isDisposed = false;

    public SourceSinkPair(WebSocketConnection source, WebSocketConnection sink, IGatewayConfiguration configuration)
    {
      Source = source;
      Sink = sink;

      source.MessageReceived += OnSourceMessageReceived;
      sink.MessageReceived += OnSinkMessageReceived;

      var message = new PairingCompleteMessage() {
        ConnectionTimeout = configuration.ConnectionTimeout,
        IceServers = configuration.IceServers.ToArray() 
      };
      source.SendMessage(message);
      sink.SendMessage(message);
    }

    public void Dispose()
    {
      if (_isDisposed) return;
      _isDisposed = true;

      Source.MessageReceived -= OnSourceMessageReceived;
      Sink.MessageReceived -= OnSinkMessageReceived;
    }

    private void OnSourceMessageReceived(object? sender, WarprMessage message)
    {
      switch (message)
      {
        case PeerConnectionDescriptionMessage:
        case PeerConnectionCandidateMessage:
          Sink.SendMessage(message);
          break;
      }
    }

    private void OnSinkMessageReceived(object? sender, WarprMessage message)
    {
      switch (message)
      {
        case PeerConnectionDescriptionMessage:
        case PeerConnectionCandidateMessage:
          Source.SendMessage(message);
          break;
      }
    }
  }

  public interface IMatchmaker
  {

  }

  public class Matchmaker : IMatchmaker
  {
    private readonly ILogger _logger;
    private readonly IStreamingSourceRepository _streamingSource;
    private readonly IStreamingSinkRepository _streamingSink;
    private readonly IGatewayConfiguration _gatewayConfiguration;

    private readonly object _syncRoot = new();

    private readonly Dictionary<string, WebSocketConnection> _freeSources = [];
    private readonly Dictionary<string, WebSocketConnection> _freeSinks = [];
    private readonly List<SourceSinkPair> _pairs = [];

    public Matchmaker(
      ILogger<Matchmaker> logger,
      IStreamingSourceRepository sourceRepository,
      IStreamingSinkRepository sinkRepository,
      IGatewayConfiguration gatewayConfiguration)
    {
      _logger = logger;
      _streamingSource = sourceRepository;
      _streamingSink = sinkRepository;
      _gatewayConfiguration = gatewayConfiguration;

      _streamingSource.ConnectionAdded += OnSourceAdded;
      _streamingSource.ConnectionRemoved += OnSourceRemoved;

      _streamingSink.ConnectionAdded += OnSinkAdded;
      _streamingSink.ConnectionRemoved += OnSinkRemoved;
    }

    private void OnSourceAdded(object? sender, WebSocketConnection e)
    {
      lock (_syncRoot)
      {
        _freeSources[e.EndPoint] = e;
        PairSourcesAndSinks();
      }
    }

    private void OnSourceRemoved(object? sender, WebSocketConnection e)
    {
      lock (_syncRoot)
      {
        if (!_freeSources.Remove(e.EndPoint))
        {
          var pair = Remove(p => p.Source == e);
          if (pair != null)
          {
            _freeSinks[pair.Sink.EndPoint] = pair.Sink;
          }
        }
      }
    }

    private void OnSinkAdded(object? sender, WebSocketConnection e)
    {
      lock (_syncRoot)
      {
        _freeSinks[e.EndPoint] = e;
        PairSourcesAndSinks();
      }
    }

    private void OnSinkRemoved(object? sender, WebSocketConnection e)
    {
      lock (_syncRoot)
      {
        if (!_freeSinks.Remove(e.EndPoint))
        {
          var pair = Remove(p => p.Sink == e);
          if (pair != null)
          {
            _freeSources[pair.Source.EndPoint] = pair.Source;
          }
        }
      }
    }

    private SourceSinkPair? Remove(Func<SourceSinkPair, bool> predicate)
    {
      var pair = _pairs.FirstOrDefault(predicate);
      if (pair != null)
      {
        _logger.LogInformation($"Pair {pair.Source.EndPoint} -> {pair.Sink.EndPoint} removed.");
        pair.Dispose();
        _pairs.Remove(pair);

        return pair;
      }

      return null;
    }

    private void PairSourcesAndSinks()
    {
      if (_freeSources.Count == 0 || _freeSinks.Count == 0) return;

      var sources = _freeSources.Values.ToArray();
      var sinks = _freeSinks.Values.ToArray();

      foreach (var source in sources)
      {
        var sink = sinks.FirstOrDefault(p => source.SessionId == Guid.Empty || p.SessionId == source.SessionId);
        if (sink == null) continue;

        _logger.LogInformation($"Pair {source.EndPoint} -> {sink.EndPoint} added.");
        _pairs.Add(new SourceSinkPair(source, sink, _gatewayConfiguration));

        _freeSources.Remove(source.EndPoint);
        _freeSinks.Remove(sink.EndPoint);
      }
    }
  }
}
