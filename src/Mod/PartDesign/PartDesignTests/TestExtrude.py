# SPDX-License-Identifier: LGPL-2.1-or-later
# UniCAD: tests for unified Extrude Join/Cut (Fusion-style Operation)

import unittest

import FreeCAD
import TestSketcherApp


class TestExtrude(unittest.TestCase):
    def setUp(self):
        self.Doc = FreeCAD.newDocument("PartDesignTestExtrude")

    def _makeBasePad(self, reversed_pad=False):
        """10x10 pad, length 10 → volume 1000."""
        body = self.Doc.addObject("PartDesign::Body", "Body")
        padSketch = self.Doc.addObject("Sketcher::SketchObject", "PadSketch")
        body.addObject(padSketch)
        TestSketcherApp.CreateRectangleSketch(padSketch, (0, 0), (10, 10))
        self.Doc.recompute()
        pad = self.Doc.addObject("PartDesign::Pad", "Pad")
        body.addObject(pad)
        pad.Profile = padSketch
        pad.Length = 10
        if reversed_pad:
            pad.Reversed = 1
        self.Doc.recompute()
        return body, pad

    def testExtrudeCutCreatesHole(self):
        """Operation=Cut must subtract from the body (hole), not leave an object-in-object."""
        # Reversed pad so material lies opposite the sketch normal; Cut reverses
        # into the solid (same setup as TestPocket).
        body, pad = self._makeBasePad(reversed_pad=True)
        self.assertAlmostEqual(pad.Shape.Volume, 1000.0)

        cutSketch = self.Doc.addObject("Sketcher::SketchObject", "CutSketch")
        body.addObject(cutSketch)
        TestSketcherApp.CreateRectangleSketch(cutSketch, (2.5, 2.5), (5, 5))
        self.Doc.recompute()

        extrude = self.Doc.addObject("PartDesign::Extrude", "ExtrudeCut")
        body.addObject(extrude)
        extrude.Profile = cutSketch
        extrude.Operation = "Cut"
        extrude.Length = 13  # into the body (Cut reverses profile normal)
        self.Doc.recompute()

        # 5x5x10 pocket through pad → remaining volume 1000 - 250 = 750
        self.assertAlmostEqual(extrude.Shape.Volume, 750.0, places=4)
        # Single solid result — not a compound of two solids
        self.assertEqual(len(extrude.Shape.Solids), 1)

    def testExtrudeJoinFuses(self):
        """Operation=Join must fuse additive material with the body."""
        body, pad = self._makeBasePad()

        joinSketch = self.Doc.addObject("Sketcher::SketchObject", "JoinSketch")
        body.addObject(joinSketch)
        # Sketch on same plane as pad base; Join extrudes outward (normal)
        TestSketcherApp.CreateRectangleSketch(joinSketch, (10, 0), (5, 5))
        self.Doc.recompute()

        extrude = self.Doc.addObject("PartDesign::Extrude", "ExtrudeJoin")
        body.addObject(extrude)
        extrude.Profile = joinSketch
        extrude.Operation = "Join"
        extrude.Length = 10
        self.Doc.recompute()

        # Original 1000 + new 5x5x10 = 1250
        self.assertAlmostEqual(extrude.Shape.Volume, 1250.0, places=4)
        self.assertEqual(len(extrude.Shape.Solids), 1)

    def testExtrudeCutDirectionReversedVsJoin(self):
        """Cut stores an opposite Direction than Join for the same profile."""
        body, _pad = self._makeBasePad(reversed_pad=True)

        sketch = self.Doc.addObject("Sketcher::SketchObject", "OpSketch")
        body.addObject(sketch)
        TestSketcherApp.CreateRectangleSketch(sketch, (2.5, 2.5), (5, 5))
        self.Doc.recompute()

        extrude = self.Doc.addObject("PartDesign::Extrude", "ExtrudeOp")
        body.addObject(extrude)
        extrude.Profile = sketch
        extrude.Operation = "Join"
        extrude.Length = 5
        self.Doc.recompute()
        joinDir = FreeCAD.Vector(extrude.Direction)

        extrude.Operation = "Cut"
        self.Doc.recompute()
        cutDir = FreeCAD.Vector(extrude.Direction)

        self.assertAlmostEqual(joinDir.dot(cutDir), -joinDir.Length ** 2, places=6)

    def tearDown(self):
        FreeCAD.closeDocument("PartDesignTestExtrude")
