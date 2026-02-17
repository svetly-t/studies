Install Fdroid

Install Termux

Copy this block into your terminal (from GPT):
```
#!/data/data/com.termux/files/usr/bin/bash
set -e

echo "== Updating packages =="
pkg update -y
pkg upgrade -y

echo "== Core tools =="
pkg install -y \
  git curl wget unzip \
  clang make cmake ninja pkg-config \
  python nodejs \
  neovim \
  ripgrep fd fzf \
  openssh

echo "== Storage access =="
termux-setup-storage || true

echo "== Git config (edit later if you want) =="
git config --global init.defaultBranch main
git config --global core.editor nvim
git config --global pull.rebase false

echo "== Useful shell aliases =="
cat >> ~/.bashrc <<'EOF'

# nicer ls
alias ls="ls --color=auto"
alias ll="ls -lah"

# git helpers
alias gs="git status"
alias ga="git add ."
alias gc="git commit -m"
alias gp="git push"
alias gl="git pull"

# ensure colors + proper terminal
export TERM=xterm-256color
EOF

echo "== Neovim config =="
mkdir -p ~/.config/nvim

cat > ~/.config/nvim/init.vim <<'EOF'
let mapleader=" "

set nocompatible
set encoding=utf-8
filetype plugin indent on
syntax on

" display
set number
set relativenumber
set cursorline
set laststatus=2
set scrolloff=5
set termguicolors

" indentation
set tabstop=2
set shiftwidth=2
set expandtab
set smartindent
set autoindent

" search
set incsearch
set ignorecase
set smartcase
set hlsearch

" usability
set backspace=indent,eol,start
set mouse=a
set splitbelow
set splitright

" fast escape for phone
inoremap jk <ESC>

" quick actions
nnoremap <leader>w :w<CR>
nnoremap <leader>q :q<CR>
nnoremap <leader>e :Ex<CR>
nnoremap <TAB> :bn<CR>
nnoremap <S-TAB> :bp<CR>

" C/C++ syntax improvements
let g:cpp_class_scope_highlight = 1
let g:cpp_member_variable_highlight = 1
let g:cpp_experimental_simple_template_highlight = 1
let g:cpp_experimental_template_highlight = 1
EOF

echo "== Install Gruvbox colors =="
mkdir -p ~/.config/nvim/colors
curl -fsSL \
  https://raw.githubusercontent.com/morhetz/gruvbox/master/colors/gruvbox.vim \
  -o ~/.config/nvim/colors/gruvbox.vim

cat >> ~/.config/nvim/init.vim <<'EOF'

set background=dark
colorscheme gruvbox

let g:gruvbox_contrast_dark='hard'
EOF

echo "== Termux extra keys (arrow keys, ctrl, esc) =="
mkdir -p ~/.termux
cat > ~/.termux/termux.properties <<'EOF'
extra-keys = [['ESC','/','-','HOME','UP','END','PGUP'],['TAB','CTRL','ALT','LEFT','DOWN','RIGHT','PGDN']]
EOF
termux-reload-settings || true

echo
echo "DONE."
echo "Restart Termux now, then test with:"
echo "  nvim"
echo "  clang --version"
echo "  git --version"
```

Set up your ssh keys in the usual way:
```
ssh-keygen -t ed25519
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/id_ed25519
```

Then clone this repo
