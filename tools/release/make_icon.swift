// Draws the app icon (an original generic star on a night-sky tile, not Nintendo artwork) as a transparent
// 1024x1024 PNG: swift make_icon.swift out.png   (tools/release/make_icon.sh turns it into AppIcon.icns)
import AppKit

let side = 1024
let rep = NSBitmapImageRep(bitmapDataPlanes: nil, pixelsWide: side, pixelsHigh: side, bitsPerSample: 8, samplesPerPixel: 4,
                           hasAlpha: true, isPlanar: false, colorSpaceName: .deviceRGB, bytesPerRow: 0, bitsPerPixel: 0)!
NSGraphicsContext.saveGraphicsState()
NSGraphicsContext.current = NSGraphicsContext(bitmapImageRep: rep)
let tile = NSBezierPath(roundedRect: NSRect(x: 100, y: 100, width: 824, height: 824), xRadius: 185, yRadius: 185)
NSGradient(starting: NSColor(red: 0.17, green: 0.23, blue: 0.56, alpha: 1), ending: NSColor(red: 0.04, green: 0.06, blue: 0.2, alpha: 1))!
    .draw(in: tile, relativeCenterPosition: NSPoint(x: 0, y: 0.25))
NSColor(white: 1, alpha: 0.85).setFill()
for (x, y, r) in [(230, 760, 9), (800, 800, 12), (860, 380, 8), (190, 320, 11), (700, 160, 9), (330, 870, 7)] {
    NSBezierPath(ovalIn: NSRect(x: x - r, y: y - r, width: 2 * r, height: 2 * r)).fill()
}
let star = NSBezierPath()
let pts = [(512, 819), (600, 616), (820, 596), (653, 452), (704, 236), (512, 348), (320, 236), (371, 452), (204, 596), (424, 616)]
star.move(to: NSPoint(x: pts[0].0, y: pts[0].1))
for p in pts.dropFirst() { star.line(to: NSPoint(x: p.0, y: p.1)) }
star.close()
star.lineJoinStyle = .round
NSGradient(starting: NSColor(red: 0.96, green: 0.72, blue: 0, alpha: 1), ending: NSColor(red: 1, green: 0.95, blue: 0.63, alpha: 1))!
    .draw(in: star, angle: 90)
NSColor(red: 0.72, green: 0.49, blue: 0, alpha: 1).setStroke()
star.lineWidth = 14
star.stroke()
NSColor(red: 0.23, green: 0.16, blue: 0, alpha: 1).setFill()
for x in [470, 554] { NSBezierPath(ovalIn: NSRect(x: x - 22, y: 524 - 22, width: 44, height: 44)).fill() }
NSGraphicsContext.restoreGraphicsState()
try! rep.representation(using: .png, properties: [:])!.write(to: URL(fileURLWithPath: CommandLine.arguments[1]))
