// See README.md for license details.

package gcd

import chisel3._
// _root_ disambiguates from package chisel3.util.circt if user imports chisel3.util._
import _root_.circt.stage.ChiselStage
import chisel3.stage.ChiselGeneratorAnnotation

/**
  * Compute GCD using subtraction method.
  * Subtracts the smaller from the larger until register y is zero.
  * value in register x is then the GCD
  */
class GCD extends Module {
  val io = IO(new Bundle {
    val value1        = Input(UInt(16.W))
    val value2        = Input(UInt(16.W))
    val loadingValues = Input(Bool())
    val outputGCD     = Output(UInt(16.W))
    val outputValid   = Output(Bool())
  })

  val x  = Reg(UInt())
  val y  = Reg(UInt())

  when(x > y) { x := x - y }
    .otherwise { y := y - x }

  when(io.loadingValues) {
    x := io.value1
    y := io.value2
  }

  io.outputGCD := x
  io.outputValid := y === 0.U
}

/**
 * Generate FIRRTL and Verilog sources
 */
object GCD extends App {
  // Generate FIRRTL
  val firrtlString = ChiselStage.emitCHIRRTL(new GCD)
  println(firrtlString)
  
  // Save FIRRTL to file
  import java.io.{File, PrintWriter}
  val firrtlFile = new File("test_run_dir/GCD.fir")
  firrtlFile.getParentFile.mkdirs()
  val writer = new PrintWriter(firrtlFile)
  writer.write(firrtlString)
  writer.close()

  // Generate SystemVerilog
  ChiselStage.emitSystemVerilogFile(
    new GCD,
    firtoolOpts = Array("-disable-all-randomization", "-strip-debug-info")
  )
}
