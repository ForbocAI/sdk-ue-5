# ForbocAI SDK Test Project

This is the standalone test/host project for the ForbocAI UE SDK. It is used by SDK contributors to drive the plugin during development, without requiring a separate game checkout.

## Structure and Discovery

The project uses `AdditionalPluginDirectories: [".forbocai-plugin-host"]` in its `.uproject` file. The runner scripts create that ignored local host as a symlink/junction to the current SDK checkout before invoking Unreal, so the project discovers exactly one `ForbocAI_SDK` plugin without manual setup.

## Running Tests

To run the test scenarios, including the parity verifier, contract harness, and automation tests:

1. Build the editor project (`ForbocAI_SDK_Editor`).
2. Run the parity verifier from the repo root:
   ```bash
   bash scripts/verify-ue-parity.sh
   ```
3. Run the commandlet-backed micro-game CLI:
   ```bash
   scripts/forbocai-ue-micro-game --mode autoplay
   ```
4. Run the SDK automation tests, optionally narrowed to a test prefix:
   ```bash
   scripts/testing/run-ue-automation-tests ForbocAI.Core.FunctionalCore
   ```

The commandlet is `ForbocAIMicroGame`. It lives in this micro-game module and delegates scenario commands through `MicroGame::CommandSurface`, which calls the SDK `CLIOps` command boundary.
