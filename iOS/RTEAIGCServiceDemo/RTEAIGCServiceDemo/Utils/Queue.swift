//
//  Queue.swift
//  RTEAIGCServiceDemo
//
//  Created by ZhouRui on 2024/6/19.
//

import Foundation

struct Queue<T> {
    private var elements: [T] = []
    private let semaphore = DispatchSemaphore(value: 1)
    private let logTag = "Queue"
    
    mutating func enqueue(_ element: T) {
        semaphore.wait()
        elements.append(element)
        semaphore.signal()
    }
    
    mutating func reset() {
        semaphore.wait()
        elements.removeAll()
        semaphore.signal()
    }
    
    mutating func dequeue() -> T? {
        semaphore.wait()
        defer { semaphore.signal() }
        let t = elements.isEmpty ? nil : elements.removeFirst()
        return t
    }
    
    func peek() -> T? {
        semaphore.wait()
        defer { semaphore.signal() }
        return elements.first
    }
    
    func isEmpty() -> Bool {
        semaphore.wait()
        defer { semaphore.signal() }
        return elements.isEmpty
    }
    
    func count() -> Int {
        semaphore.wait()
        defer { semaphore.signal() }
        return elements.count
    }
}
