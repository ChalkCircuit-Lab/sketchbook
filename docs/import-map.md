# Import Map

No old source remotes or copied project folders were available in this workspace
when the first scaffold was created. Each listed project is represented as a
placeholder and should be imported later with history preservation if a source
remote becomes available.

| Original repo | New original location | Clean target location | History preserved? | Status | Notes |
| --- | --- | --- | --- | --- | --- |
| bootstrap_demo_5 | `archive/originals/bootstrap_demo_5` | `frontend/bootstrap/bootstrap-demo-5` | No | placeholder | Source remote/folder not available during scaffold. |
| bootstrap_demo_8 | `archive/originals/bootstrap_demo_8` | `frontend/bootstrap/bootstrap-demo-8` | No | placeholder | Source remote/folder not available during scaffold. |
| bootstrap_avatar_cards_12 | `archive/originals/bootstrap_avatar_cards_12` | `frontend/bootstrap/bootstrap-avatar-cards-12` | No | placeholder | Source remote/folder not available during scaffold. |
| colored_sections | `archive/originals/colored_sections` | `frontend/html-css/colored-sections` | No | placeholder | Source remote/folder not available during scaffold. |
| clock-navbar | `archive/originals/clock-navbar` | `frontend/javascript/clock-navbar` | No | placeholder | Source remote/folder not available during scaffold. |
| React-Vite-demo | `archive/originals/React-Vite-demo` | `frontend/react/react-vite-demo` | No | placeholder | Source remote/folder not available during scaffold. |
| disney-personality-quiz | `archive/originals/disney-personality-quiz` | `frontend/javascript/disney-personality-quiz` | No | placeholder | Source remote/folder not available during scaffold. |
| Coffee-Lover | `archive/originals/Coffee-Lover` | `frontend/html-css/coffee-lover` | No | placeholder | Source remote/folder not available during scaffold. |
| JavaTech-Foundations | `archive/originals/JavaTech-Foundations` | `backend/java/javatech-foundations` | No | placeholder | Source remote/folder not available during scaffold. |
| Java-Foundations | `archive/originals/Java-Foundations` | `backend/java/java-foundations` | No | placeholder | Source remote/folder not available during scaffold. |
| javatech.lab02.config | `archive/originals/javatech.lab02.config` | `backend/java/javatech-lab02-config` | No | placeholder | Source remote/folder not available during scaffold. |
| Concurrent-Distributed-Labs | `archive/originals/Concurrent-Distributed-Labs` | `backend/distributed-systems/concurrent-distributed-labs` | No | placeholder | Source remote/folder not available during scaffold. |
| Zoo-Database | `archive/originals/Zoo-Database` | `backend/databases/zoo-database` | No | placeholder | Source remote/folder not available during scaffold. |
| Zoo-Interface | `archive/originals/Zoo-Interface` | `backend/java/zoo-interface` | No | placeholder | Source remote/folder not available during scaffold. |
| Route-Seeker | `archive/originals/Route-Seeker` | `backend/algorithms/route-seeker` | No | placeholder | Source remote/folder not available during scaffold. |

## Import Rule

If a source repository URL is found later, prefer a history-preserving import:

```bash
git remote add old-bootstrap-demo-5 <OLD_REPO_URL>
git fetch old-bootstrap-demo-5
git subtree add --prefix=archive/originals/bootstrap_demo_5 old-bootstrap-demo-5 main
```

Use `master` instead of `main` if that is the source branch. If subtree import
fails or is too fragile, copy the files into `archive/originals/<repo-name>` and
update this table with the reason history was not preserved.
